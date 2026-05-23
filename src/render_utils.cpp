#include "render_utils.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkRRect.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "src/base/SkBase64.h"
#include <algorithm>
#include <array>
#include <iostream>
#include <regex>
#include <type_traits>
#include <unordered_map>
#include <variant>

namespace common_renderer {

// Image cache implementation
namespace {
// Get the image cache - using function-local static to avoid global variable
// warning
std::unordered_map<std::string, sk_sp<SkImage>> &getImageCache() {
  static std::unordered_map<std::string, sk_sp<SkImage>> cache;
  return cache;
}

// Get the cached CRT shader effect - compile once and reuse
sk_sp<SkRuntimeEffect> getCRTShaderEffect() {
  static sk_sp<SkRuntimeEffect> cachedEffect = nullptr;

  if (!cachedEffect) {
    constexpr const char *CRT_SHADER_CODE = R"(
      uniform shader child;
      uniform float verticalBleed;
      uniform float horizontalBleed;
      uniform float contrast;
      uniform float granularity;
      uniform float2 shaderSize;
      uniform float2 shaderOffset;

      // sRGB to Linear
      half ToLinear1(half c) {
        return (c <= 0.04045) ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4);
      }
      half3 ToLinear(half3 c) {
        return half3(ToLinear1(c.r), ToLinear1(c.g), ToLinear1(c.b));
      }

      // Linear to sRGB
      half ToSrgb1(half c) {
        return (c < 0.0031308) ? c * 12.92 : 1.055 * pow(c, 0.41666) - 0.055;
      }
      half3 ToSrgb(half3 c) {
        return half3(ToSrgb1(c.r), ToSrgb1(c.g), ToSrgb1(c.b));
      }

      // Sample the input image and linearize the color space
      half4 SampleAndLinearize(half2 pos, half2 offset) {
        half2 res = shaderSize / granularity;
        pos = floor(pos * res + offset) * granularity;
        // Check if pos is within bounds [0, shaderSize.x] and [0, shaderSize.y]
        if (pos.x < 0.0 || pos.x >= shaderSize.x || pos.y < 0.0 || pos.y >= shaderSize.y) {
          return half4(0.0, 0.0, 0.0, 0.0);
        }
        half4 sample = child.eval(pos + shaderOffset);
        return half4(ToLinear(sample.rgb), sample.a);
      }

      // Distance in emulated pixels to nearest texel
      half2 Dist(half2 pos) {
        half2 res = shaderSize / granularity;
        pos = pos * res;
        return -((pos - floor(pos)) - half2(0.5));
      }

      // 1D Gaussian
      half Gaussian(half pos, half stddev) {
        return exp2(- pos * pos / (2.0 * stddev * stddev));
      }

      // 3-pixel Gaussian filter along horz line
      half4 Horz3(half2 pos, half offsetY) {
        half4 sampleLeft = SampleAndLinearize(pos, half2(-1.0, offsetY));
        half4 sampleCenter = SampleAndLinearize(pos, half2(0.0, offsetY));
        half4 sampleRight = SampleAndLinearize(pos, half2(1.0, offsetY));
        half distX = Dist(pos).x;
        half weightLeft = Gaussian(distX - 1.0, horizontalBleed);
        half weightCenter = Gaussian(distX + 0.0, horizontalBleed);
        half weightRight = Gaussian(distX + 1.0, horizontalBleed);
        half totalWeight = weightLeft + weightCenter + weightRight;
        return (sampleLeft * weightLeft + sampleCenter * weightCenter + sampleRight * weightRight) / totalWeight;
      }

      // 5-pixel Gaussian filter along horz line
      half4 Horz5(half2 pos, half offsetY) {
        half4 sampleLeft2 = SampleAndLinearize(pos, half2(-2.0, offsetY));
        half4 sampleLeft1 = SampleAndLinearize(pos, half2(-1.0, offsetY));
        half4 sampleCenter = SampleAndLinearize(pos, half2(0.0, offsetY));
        half4 sampleRight1 = SampleAndLinearize(pos, half2(1.0, offsetY));
        half4 sampleRight2 = SampleAndLinearize(pos, half2(2.0, offsetY));
        half distX = Dist(pos).x;
        half weightLeft2 = Gaussian(distX - 2.0, horizontalBleed);
        half weightLeft1 = Gaussian(distX - 1.0, horizontalBleed);
        half weightCenter = Gaussian(distX + 0.0, horizontalBleed);
        half weightRight1 = Gaussian(distX + 1.0, horizontalBleed);
        half weightRight2 = Gaussian(distX + 2.0, horizontalBleed);
        half totalWeight = weightLeft2 + weightLeft1 + weightCenter + weightRight1 + weightRight2;
        return (sampleLeft2 * weightLeft2 + sampleLeft1 * weightLeft1 + 
                sampleCenter * weightCenter + sampleRight1 * weightRight1 + 
                sampleRight2 * weightRight2) / totalWeight;
      }

      half ScanLineWeight(half2 pos, half offsetY) {
        half dst = Dist(pos).y;
        return Gaussian(dst + offsetY, verticalBleed);
      }

      // Simulate CRT scanline color bleeding
      half4 BleedColor(half2 pos) {
        half4 top = Horz3(pos, -1.0);
        half4 center = Horz5(pos, 0.0);
        half4 bottom = Horz3(pos, 1.0);
        half weightTop = ScanLineWeight(pos, -1.0);
        half weightCenter = ScanLineWeight(pos, 0.0);
        half weightBottom = ScanLineWeight(pos, 1.0);
        half totalWeight = weightTop + weightCenter + weightBottom;
        return (top * weightTop + center * weightCenter + bottom * weightBottom) / totalWeight;
      }

      // Simulate CRT Shadow mask
      // for every 3 pixels in a row, highlight one color channel RGB
      // offset the next row by 1 pixels, so next row will become BRG
      half3 Mask(half2 coord) {
        coord.x += coord.y;
        coord.x = fract(coord.x / 3.0);
        // Branchless: use step() to determine which channel is bright
        // step(edge, x) returns 1.0 if x >= edge, 0.0 otherwise
        // Since coord.x is from fract(), it's always in [0, 1)
        half isRed = 1.0 - step(0.333, coord.x);
        half isGreen = step(0.333, coord.x) * (1.0 - step(0.666, coord.x));
        half isBlue = step(0.666, coord.x);
        // Calculate maskDark and maskLight from contrast
        half maskDark = 1.0 - contrast;
        half maskLight = 1.0 + contrast;
        return half3(
          mix(maskDark, maskLight, isRed),
          mix(maskDark, maskLight, isGreen),
          mix(maskDark, maskLight, isBlue)
        );
      }

      half4 main(float2 coord) {
        coord = coord - shaderOffset;
        half2 pos = coord/shaderSize;
        half4 color = BleedColor(pos);
        half3 rgb = ToSrgb(color.rgb * Mask(half2(coord)));
        return half4(rgb, color.a);
      }
    )";

    auto [effect, errorText] =
        SkRuntimeEffect::MakeForShader(SkString(CRT_SHADER_CODE));
    if (!effect) {
      std::cerr << "Failed to compile CRT shader: " << errorText.c_str()
                << std::endl;
      return nullptr;
    }
    cachedEffect = effect;
  }

  return cachedEffect;
}

// Get the cached LiquidGlass shader effect - compile once and reuse
sk_sp<SkRuntimeEffect> getLiquidGlassShaderEffect() {
  static sk_sp<SkRuntimeEffect> cachedEffect = nullptr;

  if (!cachedEffect) {
    constexpr const char *LIQUID_GLASS_SHADER_CODE = R"(
      uniform shader child;
      uniform float rim;
      uniform float refraction;
      uniform float tint;
      uniform float4 cornerRadii; // [top-left, top-right, bottom-right, bottom-left]
      uniform float2 shaderSize;
      uniform float2 shaderOffset;

      // SDF of an ellipse/oval.
      // center: center of the ellipse
      // size: half-width and half-height (radii) of the ellipse
      // p: point to evaluate
      half sdfOval(half2 center, half2 size, half2 p) {
        half k0 = length((p - center) / size);
        half k1 = length((p - center) / (size * size));
        return k0 * (k0 - 1.0) / k1;
      }

      // SDF of a rounded rectangle with different corner radii.
      // Based on https://iquilezles.org/articles/distfunctions/
      // cornerRadii: [top-left, top-right, bottom-right, bottom-left]
      half sdfRect(half2 center, half2 size, half2 p, half4 cornerRadii) {
        half2 p_rel = p - center;
        half2 abs_p = abs(p_rel);
        
        // Determine which corner region we're in and select the appropriate radius
        // Corners are: top-left (x<0, y<0), top-right (x>=0, y<0), 
        //              bottom-right (x>=0, y>=0), bottom-left (x<0, y>=0)
        // Use step() for branchless selection: step(edge, x) returns 1.0 if x >= edge, 0.0 otherwise
        half isRight = step(0.0, p_rel.x);  // 1 if x >= 0, 0 if x < 0
        half isBottom = step(0.0, p_rel.y); // 1 if y >= 0, 0 if y < 0
        
        // Select corner radius based on quadrant
        // Top-left: !isRight && !isBottom -> (1-isRight) * (1-isBottom)
        // Top-right: isRight && !isBottom -> isRight * (1-isBottom)
        // Bottom-right: isRight && isBottom -> isRight * isBottom
        // Bottom-left: !isRight && isBottom -> (1-isRight) * isBottom
        half r = cornerRadii.x * (1.0 - isRight) * (1.0 - isBottom) +
                 cornerRadii.y * isRight * (1.0 - isBottom) +
                 cornerRadii.z * isRight * isBottom +
                 cornerRadii.w * (1.0 - isRight) * isBottom;
        
        half2 q = abs_p - size + r;
        return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
      }

      // Rim is the t in the doc.
      half3 getNormal(half2 center, half2 halfRectSize, half2 coordLayer, half4 cornerRadii, half rim, half isOval) {
        // Compute gradient manually using finite differences
        half eps = 0.5;
        half sd_x0, sd_x1, sd_y0, sd_y1, sd;
        
        if (isOval > 0.5) {
          // Use oval SDF
          sd_x0 = sdfOval(center, halfRectSize, coordLayer - half2(eps, 0.0));
          sd_x1 = sdfOval(center, halfRectSize, coordLayer + half2(eps, 0.0));
          sd_y0 = sdfOval(center, halfRectSize, coordLayer - half2(0.0, eps));
          sd_y1 = sdfOval(center, halfRectSize, coordLayer + half2(0.0, eps));
          sd = sdfOval(center, halfRectSize, coordLayer);
        } else {
          // Use rounded rectangle SDF
          sd_x0 = sdfRect(center, halfRectSize, coordLayer - half2(eps, 0.0), cornerRadii);
          sd_x1 = sdfRect(center, halfRectSize, coordLayer + half2(eps, 0.0), cornerRadii);
          sd_y0 = sdfRect(center, halfRectSize, coordLayer - half2(0.0, eps), cornerRadii);
          sd_y1 = sdfRect(center, halfRectSize, coordLayer + half2(0.0, eps), cornerRadii);
          sd = sdfRect(center, halfRectSize, coordLayer, cornerRadii);
        }
        
        half dx = (sd_x1 - sd_x0) / (2.0 * eps);
        half dy = (sd_y1 - sd_y0) / (2.0 * eps);
        
        // The cosine and sine between normal and the xy plane.
        half n_cos = max(rim + sd, 0.0) / rim;
        half n_sin = sqrt(1.0 - n_cos * n_cos);
        return normalize(half3(dx * n_cos, dy * n_cos, n_sin));
      }

      // The height (z component) of the pad surface at sd.
      half padHeight(half sd, half rim) {
        if (sd >= 0.0) {
          return 0.0;
        }
        if (sd < -rim) {
          return rim;
        }
        half x = rim + sd;
        return sqrt(rim * rim - x * x);
      }

      half4 main(float2 coord) {
        // coord is in canvas coordinates
        // For SDF calculations, we need layer-relative coordinates
        half2 coordLayer = coord - shaderOffset;
        
        // Check if cornerRadii are negative (indicates oval)
        // If any corner radius is negative, treat as oval
        half isOval = step(0.0, -cornerRadii.x - 0.0001); // 1.0 if cornerRadii.x < 0, 0.0 otherwise
        
        // Calculate SDF - use oval if radii are negative, otherwise use rounded rectangle
        // halfRectSize is half-width and half-height (the SDF function expects half-size)
        half2 halfRectSize = shaderSize * 0.5;
        // center and halfRectSize are the same numerically
        half sd = mix(
          sdfRect(halfRectSize, halfRectSize, coordLayer, cornerRadii),
          sdfOval(halfRectSize, halfRectSize, coordLayer),
          isOval
        );
        
        // Background pass-through with anti-aliasing
        half aaFactor = clamp(sd / 100.0, 0.0, 1.0) * 0.1 + 0.9;
        half4 bg_sample = child.eval(coord);
        half4 bg_col = mix(half4(0.0), bg_sample, aaFactor);
        bg_col.a = smoothstep(-4.0, 0.0, sd);
        
        half3 normal = getNormal(halfRectSize, halfRectSize, coordLayer, cornerRadii, rim, isOval);
        
        // A ray going -z hits the top of the pad, where would it hit on
        // the z = -thickness plane? (thickness = rim * 8)
        half thickness = rim * 8.0;
        half3 incident = half3(0.0, 0.0, -1.0); // Should be normalized.
        half3 refract_vec = refract(incident, normal, 1.0 / refraction);
        half h = padHeight(sd, rim);
        half refract_length = (h + thickness) / dot(half3(0.0, 0.0, -1.0), refract_vec);
        
        // This is the screen coord of the ray hitting the z = -thickness plane.
        // refract_vec.xy is in layer space, so add it to coordLayer, then convert back to canvas coords
        half2 image_coord = coordLayer + refract_vec.xy * refract_length + shaderOffset;
        
        // Sample background at refracted position
        half4 refract_color = child.eval(image_coord);
        
        // Reflection: whiteness around edges
        half3 reflect_vec = reflect(incident, normal);
        // x ~= y: darker around 45 degree corners
        // x != y: brighter around horizontal and vertical edges
        half c = clamp(abs(reflect_vec.x - reflect_vec.y), 0.0, 1.0);
        half4 reflect_color = half4(c, c, c, 0.0);
        
        // Mix refraction and reflection
        half reflectFactor = 1.0 - normal.z;
        half4 glass_color = mix(refract_color, reflect_color, reflectFactor);
        
        // Mix with glass tint
        half4 glass_tint = half4(0.1, 0.1, 0.1, 0.0);
        glass_color = mix(glass_color, glass_tint, tint);
        
        // Mix with bg for anti-aliasing
        glass_color = clamp(glass_color, 0.0, 1.0);
        bg_col = clamp(bg_col, 0.0, 1.0);
        return mix(glass_color, bg_col, bg_col.a);
      }
    )";

    auto [effect, errorText] =
        SkRuntimeEffect::MakeForShader(SkString(LIQUID_GLASS_SHADER_CODE));
    if (!effect) {
      std::cerr << "Failed to compile LiquidGlass shader: " << errorText.c_str()
                << std::endl;
      return nullptr;
    }
    cachedEffect = effect;
  }

  return cachedEffect;
}
} // namespace

// Get the cached MoveIn shader effect - target slides in, source stays put
sk_sp<SkRuntimeEffect> getMoveInShaderEffect() {
  static sk_sp<SkRuntimeEffect> cachedEffect = nullptr;

  if (!cachedEffect) {
    constexpr const char *MOVE_IN_SHADER_CODE = R"(
      uniform shader sourceImage;
      uniform shader targetImage;
      uniform float progress;
      uniform float2 direction;
      uniform float2 shaderSize;

      half4 main(float2 coord) {
        vec2 uv = coord / shaderSize;
        
        // Target image slides in from the direction
        // direction (0,1) = down means target starts above and slides down
        // direction (1,0) = right means target starts left and slides right
        // At progress=0: target is fully outside the view
        // At progress=1: target is fully in view
        
        vec2 targetOffset = direction * (1.0 - progress);
        
        // Calculate where we would sample from the target
        vec2 targetUV = uv - targetOffset;
        
        // Sample both images
        half4 source = sourceImage.eval(coord);
        half4 target = targetImage.eval(targetUV * shaderSize);
        
        // Compute mix factor: 1.0 if pixel is within target bounds, 0.0 otherwise
        float mixFactor = step(0.0, targetUV.x) * step(targetUV.x, 1.0) * 
                          step(0.0, targetUV.y) * step(targetUV.y, 1.0);
        
        return mix(source, target, mixFactor);
      }
    )";

    auto [effect, errorText] =
        SkRuntimeEffect::MakeForShader(SkString(MOVE_IN_SHADER_CODE));
    if (!effect) {
      std::cerr << "Failed to compile MoveIn shader: " << errorText.c_str()
                << std::endl;
      return nullptr;
    }
    cachedEffect = effect;
  }

  return cachedEffect;
}

// Get the cached MoveOut shader effect - source slides out, target stays put
sk_sp<SkRuntimeEffect> getMoveOutShaderEffect() {
  static sk_sp<SkRuntimeEffect> cachedEffect = nullptr;

  if (!cachedEffect) {
    constexpr const char *MOVE_OUT_SHADER_CODE = R"(
      uniform shader sourceImage;
      uniform shader targetImage;
      uniform float progress;
      uniform float2 direction;
      uniform float2 shaderSize;

      half4 main(float2 coord) {
        vec2 uv = coord / shaderSize;
        
        // Source image slides out in the direction
        // direction (0,1) = down means source slides down and out
        // direction (1,0) = right means source slides right and out
        // At progress=0: source is fully visible
        // At progress=1: source is fully outside, target fully visible
        
        vec2 sourceOffset = direction * progress;
        
        // Calculate where we would sample from the source
        vec2 sourceUV = uv - sourceOffset;
        
        // Sample both images
        half4 source = sourceImage.eval(sourceUV * shaderSize);
        half4 target = targetImage.eval(coord);
        
        // Compute mix factor: 1.0 if pixel is within source bounds, 0.0 otherwise
        float mixFactor = step(0.0, sourceUV.x) * step(sourceUV.x, 1.0) * 
                          step(0.0, sourceUV.y) * step(sourceUV.y, 1.0);
        
        return mix(target, source, mixFactor);
      }
    )";

    auto [effect, errorText] =
        SkRuntimeEffect::MakeForShader(SkString(MOVE_OUT_SHADER_CODE));
    if (!effect) {
      std::cerr << "Failed to compile MoveOut shader: " << errorText.c_str()
                << std::endl;
      return nullptr;
    }
    cachedEffect = effect;
  }

  return cachedEffect;
}

std::vector<float> toTlwhPriorRotation(const std::shared_ptr<AnyNode> &jsonPtr,
                                       float parentWidth, float parentHeight) {
  if (!jsonPtr) {
    return {0, 0, parentWidth, parentHeight};
  }

  // Cast to appropriate derived class to access alignment and positioning
  // properties
  Positioning *positioning = nullptr;

  switch (jsonPtr->type) {
  case NodeType::PARAGRAPH:
    positioning = dynamic_cast<ParagraphNode *>(jsonPtr.get());
    break;
  case NodeType::LAYER:
    positioning = dynamic_cast<LayerNode *>(jsonPtr.get());
    break;
  case NodeType::SCREEN:
    positioning = dynamic_cast<ScreenNode *>(jsonPtr.get());
    break;
  case NodeType::SVG:
    positioning = dynamic_cast<SVGNode *>(jsonPtr.get());
    break;
  case NodeType::PATH:
    positioning = dynamic_cast<PathNode *>(jsonPtr.get());
    break;
  default:
    return {0, 0, parentWidth, parentHeight};
  }

  if (!positioning) {
    return {0, 0, parentWidth, parentHeight};
  }

  float x = 0, y = 0, w = parentWidth, h = parentHeight;

  // Calculate vertical offset (y)
  if (positioning->alignV == VerticalAlign::TOP ||
      positioning->alignV == VerticalAlign::EDGE) {
    if (positioning->top.has_value()) {
      auto [topValue, topIsPct] = positioning->top.value();
      y = topValue * (topIsPct ? parentHeight / 100.0F : 1.0F);
    }
  } else if (positioning->alignV == VerticalAlign::CENTER) {
    float height = parentHeight;
    if (positioning->height.has_value()) {
      auto [heightValue, heightIsPct] = positioning->height.value();
      height = heightValue * (heightIsPct ? parentHeight / 100.0F : 1.0F);
    }
    y = parentHeight / 2.0F - height / 2.0F;
  } else if (positioning->alignV == VerticalAlign::BOTTOM) {
    float height = parentHeight;
    if (positioning->height.has_value()) {
      auto [heightValue, heightIsPct] = positioning->height.value();
      height = heightValue * (heightIsPct ? parentHeight / 100.0F : 1.0F);
    }
    float bottom = 0.0F;
    if (positioning->bottom.has_value()) {
      auto [bottomValue, bottomIsPct] = positioning->bottom.value();
      bottom = bottomValue * (bottomIsPct ? parentHeight / 100.0F : 1.0F);
    }
    y = parentHeight - bottom - height;
  }

  // Calculate horizontal offset (x)
  if (positioning->alignH == HorizontalAlign::LEFT ||
      positioning->alignH == HorizontalAlign::EDGE) {
    if (positioning->left.has_value()) {
      auto [leftValue, leftIsPct] = positioning->left.value();
      x = leftValue * (leftIsPct ? parentWidth / 100.0F : 1.0F);
    }
  } else if (positioning->alignH == HorizontalAlign::CENTER) {
    float width = parentWidth;
    if (positioning->width.has_value()) {
      auto [widthValue, widthIsPct] = positioning->width.value();
      width = widthValue * (widthIsPct ? parentWidth / 100.0F : 1.0F);
    }
    x = parentWidth / 2.0F - width / 2.0F;
  } else if (positioning->alignH == HorizontalAlign::RIGHT) {
    float width = parentWidth;
    if (positioning->width.has_value()) {
      auto [widthValue, widthIsPct] = positioning->width.value();
      width = widthValue * (widthIsPct ? parentWidth / 100.0F : 1.0F);
    }
    float right = 0.0F;
    if (positioning->right.has_value()) {
      auto [rightValue, rightIsPct] = positioning->right.value();
      right = rightValue * (rightIsPct ? parentWidth / 100.0F : 1.0F);
    }
    x = parentWidth - right - width;
  }

  // Calculate height (h)
  if (positioning->alignV == VerticalAlign::EDGE) {
    float top = 0.0F;
    if (positioning->top.has_value()) {
      auto [topValue, topIsPct] = positioning->top.value();
      top = topValue * (topIsPct ? parentHeight / 100.0F : 1.0F);
    }
    float bottom = 0.0F;
    if (positioning->bottom.has_value()) {
      auto [bottomValue, bottomIsPct] = positioning->bottom.value();
      bottom = bottomValue * (bottomIsPct ? parentHeight / 100.0F : 1.0F);
    }
    h = parentHeight - top - bottom;
  } else if (positioning->height.has_value()) {
    auto [heightValue, heightIsPct] = positioning->height.value();
    h = heightValue * (heightIsPct ? parentHeight / 100.0F : 1.0F);
  }

  // Calculate width (w)
  if (positioning->alignH == HorizontalAlign::EDGE) {
    float left = 0.0F;
    if (positioning->left.has_value()) {
      auto [leftValue, leftIsPct] = positioning->left.value();
      left = leftValue * (leftIsPct ? parentWidth / 100.0F : 1.0F);
    }
    float right = 0.0F;
    if (positioning->right.has_value()) {
      auto [rightValue, rightIsPct] = positioning->right.value();
      right = rightValue * (rightIsPct ? parentWidth / 100.0F : 1.0F);
    }
    w = parentWidth - left - right;
  } else if (positioning->width.has_value()) {
    auto [widthValue, widthIsPct] = positioning->width.value();
    w = widthValue * (widthIsPct ? parentWidth / 100.0F : 1.0F);
  }

  return {x, y, w, h};
}

// Helper function to parse color string to SkColor
SkColor parseColor(const std::string &colorStr) {
  if (colorStr.empty()) {
    return SK_ColorTRANSPARENT;
  }

  // Handle hex colors like "#RRGGBB" or "#RRGGBBAA" using regex
  std::regex hex6Regex(R"(#[0-9a-fA-F]{6})");
  std::regex hex8Regex(R"(#[0-9a-fA-F]{8})");

  if (std::regex_match(colorStr, hex6Regex)) {
    std::string hex = colorStr.substr(1);
    unsigned int color = std::stoul(hex, nullptr, 16);
    return SkColorSetRGB((color >> 16U) & 0xFFU, (color >> 8U) & 0xFFU,
                         color & 0xFFU);
  } else if (std::regex_match(colorStr, hex8Regex)) {
    std::string hex = colorStr.substr(1);
    unsigned int color = std::stoul(hex, nullptr, 16);
    return SkColorSetARGB(color & 0xFFU, (color >> 24U) & 0xFFU,
                          (color >> 16U) & 0xFFU, (color >> 8U) & 0xFFU);
  }

  // Handle rgb() formats using regex
  std::regex rgbRegex(R"(rgb\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\))");
  std::smatch matches;

  if (std::regex_match(colorStr, matches, rgbRegex)) {
    int r = std::stoi(matches[1].str());
    int g = std::stoi(matches[2].str());
    int b = std::stoi(matches[3].str());
    return SkColorSetRGB(r, g, b);
  }

  return SK_ColorTRANSPARENT; // Default fallback
}

// Helper function to create SkPaint from Color variant
SkPaint createPaintFromColor(const std::optional<Color> &color, float offsetX,
                             float offsetY, float width, float height) {
  if (!color.has_value()) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorTRANSPARENT); // Default color
    return paint;
  }

  // Handle the variant using std::visit
  return std::visit(
      [&](auto &&colorValue) -> SkPaint {
        using T = std::decay_t<decltype(colorValue)>;
        SkPaint resultPaint;
        resultPaint.setAntiAlias(true);

        if constexpr (std::is_same_v<T, SolidColor>) {
          // Solid color
          resultPaint.setColor(parseColor(colorValue.color));

          // Apply shadows if specified
          if (!colorValue.shadows.empty()) {
            sk_sp<SkImageFilter> combinedFilter = nullptr;
            for (const auto &shadow : colorValue.shadows) {
              SkColor shadowColor = parseColor(shadow.color);
              sk_sp<SkImageFilter> shadowFilter;
              if (shadow.type == "innerShadow") {
                shadowFilter = createImageFilterForInnerShadow(
                    combinedFilter, shadowColor, shadow.offsetX, shadow.offsetY,
                    shadow.blurX, shadow.blurY);
              } else {
                shadowFilter = SkImageFilters::DropShadow(
                    shadow.offsetX, shadow.offsetY, shadow.blurX, shadow.blurY,
                    shadowColor, combinedFilter);
              }
              if (shadowFilter) {
                combinedFilter = shadowFilter;
              }
            }
            if (combinedFilter) {
              resultPaint.setImageFilter(combinedFilter);
            }
          }
        } else if constexpr (std::is_same_v<T, GradientColor>) {
          // Gradient color
          SkColor startColor = parseColor(colorValue.startColor);
          SkColor endColor = parseColor(colorValue.endColor);

          // Create gradient points
          std::array<SkPoint, 2> points = {
              {{colorValue.startPoint.x * width,
                colorValue.startPoint.y * height},
               {colorValue.endPoint.x * width,
                colorValue.endPoint.y * height}}};

          // Create color array
          std::array<SkColor, 2> colors = {startColor, endColor};

          // Create linear gradient shader
          sk_sp<SkShader> gradientShader = SkGradientShader::MakeLinear(
              points.data(), colors.data(), nullptr, 2, SkTileMode::kClamp);

          if (gradientShader) {
            resultPaint.setShader(gradientShader);
          } else {
            // Fallback to solid color if gradient creation fails
            resultPaint.setColor(startColor);
          }

          // Apply shadows if specified
          if (!colorValue.shadows.empty()) {
            sk_sp<SkImageFilter> combinedFilter = nullptr;
            for (const auto &shadow : colorValue.shadows) {
              SkColor shadowColor = parseColor(shadow.color);
              sk_sp<SkImageFilter> shadowFilter;
              if (shadow.type == "innerShadow") {
                shadowFilter = createImageFilterForInnerShadow(
                    combinedFilter, shadowColor, shadow.offsetX, shadow.offsetY,
                    shadow.blurX, shadow.blurY);
              } else {
                shadowFilter = SkImageFilters::DropShadow(
                    shadow.offsetX, shadow.offsetY, shadow.blurX, shadow.blurY,
                    shadowColor, combinedFilter);
              }
              if (shadowFilter) {
                combinedFilter = shadowFilter;
              }
            }
            if (combinedFilter) {
              resultPaint.setImageFilter(combinedFilter);
            }
          }
        }

        return resultPaint;
      },
      color.value());
}

// Helper function to create color filter from ImageFill filter values
sk_sp<SkColorFilter>
createColorFilterFromImageFill(const ImageFill &imageFill) {
  sk_sp<SkColorFilter> colorFilter = nullptr;
  bool hasFilters = false;

  // Exposure: scales RGB channels (exposure is in range -1 to 1)
  if (imageFill.exposure.has_value() && *imageFill.exposure != 0.0F) {
    float exposure = *imageFill.exposure;
    float factor =
        std::pow(2.0F, exposure); // Convert exposure to brightness multiplier
    SkColorMatrix exposureMatrix;
    exposureMatrix.setScale(factor, factor, factor, 1.0F);
    colorFilter = SkColorFilters::Matrix(exposureMatrix);
    hasFilters = true;
  }

  // Contrast: adjusts contrast (contrast is in range -1 to 1)
  if (imageFill.contrast.has_value() && *imageFill.contrast != 0.0F) {
    float contrast = *imageFill.contrast;
    float factor = (1.0F + 0.8F * contrast) /
                   (1.0F - 0.8F * contrast); // scale to [-0.8, 0.8]
    SkColorMatrix contrastMatrix;
    contrastMatrix.setScale(factor, factor, factor, 1.0F);
    contrastMatrix.postTranslate(0.5F * (1.0F - factor), 0.5F * (1.0F - factor),
                                 0.5F * (1.0F - factor), 0.0F);
    if (colorFilter) {
      colorFilter = SkColorFilters::Compose(
          colorFilter, SkColorFilters::Matrix(contrastMatrix));
    } else {
      colorFilter = SkColorFilters::Matrix(contrastMatrix);
    }
    hasFilters = true;
  }

  // Saturation: adjusts color saturation (saturation is in range -1 to 1)
  if (imageFill.saturation.has_value() && *imageFill.saturation != 0.0F) {
    float saturation = *imageFill.saturation;
    SkColorMatrix saturationMatrix;
    saturationMatrix.setSaturation(1.0F + saturation);
    if (colorFilter) {
      colorFilter = SkColorFilters::Compose(
          colorFilter, SkColorFilters::Matrix(saturationMatrix));
    } else {
      colorFilter = SkColorFilters::Matrix(saturationMatrix);
    }
    hasFilters = true;
  }

  // Temperature: adjusts color temperature (temperature is in range -1 to 1)
  // Positive = warmer (more red/yellow), Negative = cooler (more blue)
  if (imageFill.temperature.has_value() && *imageFill.temperature != 0.0F) {
    float temp = *imageFill.temperature;
    SkColorMatrix tempMatrix;
    // Warm: increase red, decrease blue
    // Cool: decrease red, increase blue
    float redAdjust = temp * 0.2F;
    float blueAdjust = temp * 0.2F;
    tempMatrix.setScale(1.0F + redAdjust, 1.0F, 1.0F + blueAdjust, 1.0F);
    if (colorFilter) {
      colorFilter = SkColorFilters::Compose(colorFilter,
                                            SkColorFilters::Matrix(tempMatrix));
    } else {
      colorFilter = SkColorFilters::Matrix(tempMatrix);
    }
    hasFilters = true;
  }

  // Tint: adjusts tint (tint is in range -1 to 1)
  // Positive = more green, Negative = more magenta
  if (imageFill.tint.has_value() && *imageFill.tint != 0.0F) {
    float tint = *imageFill.tint;
    SkColorMatrix tintMatrix;
    // Green: increase green, decrease red + blue
    // Magenta: decrease green, increase red + blue
    float redAdjust = -tint * 0.1F;
    float greenAdjust = -tint * 0.2F;
    float blueAdjust = -tint * 0.1F;
    tintMatrix.setScale(1.0F + redAdjust, 1.0F + greenAdjust, 1.0F + blueAdjust,
                        1.0F);
    if (colorFilter) {
      colorFilter = SkColorFilters::Compose(colorFilter,
                                            SkColorFilters::Matrix(tintMatrix));
    } else {
      colorFilter = SkColorFilters::Matrix(tintMatrix);
    }
    hasFilters = true;
  }

  // Highlights: adjusts highlights (highlights is in range -1 to 1)
  // Positive = draker highlights, Negative = brighter highlights
  if (imageFill.highlights.has_value() && *imageFill.highlights != 0.0F) {
    float highlights = *imageFill.highlights;
    // Apply a curve that affects brighter pixels more
    SkColorMatrix highlightsMatrix;
    float factor = 1.0F - 0.2F * highlights;
    highlightsMatrix.setScale(factor, factor, factor, 1.0F);
    highlightsMatrix.postTranslate(highlights * 0.02F, highlights * 0.02F,
                                   highlights * 0.02F, 0.0F);
    if (colorFilter) {
      colorFilter = SkColorFilters::Compose(
          colorFilter, SkColorFilters::Matrix(highlightsMatrix));
    } else {
      colorFilter = SkColorFilters::Matrix(highlightsMatrix);
    }
    hasFilters = true;
  }

  // Shadows: adjusts shadows (shadows is in range -1 to 1)
  // Positive = brighter shadows, Negative = darker shadows
  if (imageFill.shadows.has_value() && *imageFill.shadows != 0.0F) {
    float shadows = *imageFill.shadows;
    // Apply a curve that affects darker pixels more
    SkColorMatrix shadowsMatrix;
    float factor = 1.0F - shadows * 0.3F;
    shadowsMatrix.setScale(factor, factor, factor, 1.0F);
    shadowsMatrix.postTranslate(shadows * 0.2F, shadows * 0.2F, shadows * 0.2F,
                                0.0F);
    if (colorFilter) {
      colorFilter = SkColorFilters::Compose(
          colorFilter, SkColorFilters::Matrix(shadowsMatrix));
    } else {
      colorFilter = SkColorFilters::Matrix(shadowsMatrix);
    }
    hasFilters = true;
  }

  return hasFilters ? colorFilter : nullptr;
}

// Helper function to create ImageFilter for inner shadow
sk_sp<SkImageFilter> createImageFilterForInnerShadow(sk_sp<SkImageFilter> input,
                                                     SkColor color, float dx,
                                                     float dy, float sigmaX,
                                                     float sigmaY) {
  // backgroundPreserveFilter: outputs the background/destination content
  // unchanged
  sk_sp<SkColorFilter> backgroundPreserveCF =
      SkColorFilters::Blend(SK_ColorBLACK, SkBlendMode::kDst);
  sk_sp<SkImageFilter> backgroundPreserveFilter =
      SkImageFilters::ColorFilter(backgroundPreserveCF, nullptr);

  // foregroundAlphaFilter: outputs the alpha channel of the foreground/source
  // as a grayscale image
  sk_sp<SkColorFilter> foregroundAlphaCF =
      SkColorFilters::Blend(SK_ColorBLACK, SkBlendMode::kSrcIn);
  sk_sp<SkImageFilter> foregroundAlphaFilter =
      SkImageFilters::ColorFilter(foregroundAlphaCF, nullptr);

  // shadowColorFilter: outputs the shadow color in areas outside the source
  // shape
  sk_sp<SkColorFilter> shadowColorCF =
      SkColorFilters::Blend(color, SkBlendMode::kSrcOut);
  sk_sp<SkImageFilter> shadowColorFilter =
      SkImageFilters::ColorFilter(shadowColorCF, nullptr);

  // offsetShadowFilter: outputs the shadow color shifted by (dx, dy) offset
  sk_sp<SkImageFilter> offsetShadowFilter =
      SkImageFilters::Offset(dx, dy, shadowColorFilter);

  // blurredShadowFilter: outputs the offset shadow with blur applied (sigmaX,
  // sigmaY)
  sk_sp<SkImageFilter> blurredShadowFilter = SkImageFilters::Blur(
      sigmaX, sigmaY, SkTileMode::kDecal, offsetShadowFilter);

  // alphaMaskedShadowFilter: outputs the blurred shadow masked to only appear
  // within the foreground alpha shape
  sk_sp<SkImageFilter> alphaMaskedShadowFilter = SkImageFilters::Blend(
      SkBlendMode::kSrcIn, foregroundAlphaFilter, blurredShadowFilter);

  // blendResult: outputs the background content with the alpha-masked shadow
  // composited on top
  sk_sp<SkImageFilter> blendResult = SkImageFilters::Blend(
      SkBlendMode::kSrcOver, backgroundPreserveFilter, alphaMaskedShadowFilter);
  // Returns: outputs the input filter composited with the inner shadow effect
  return SkImageFilters::Compose(input, blendResult);
}

// Helper function to create SkPaint from Fill variant
SkPaint createPaintFromFill(const Fill &fill, float offsetX, float offsetY,
                            float width, float height) {
  // Handle the variant using std::visit
  return std::visit(
      [&](auto &&fillValue) -> SkPaint {
        using T = std::decay_t<decltype(fillValue)>;
        if constexpr (std::is_same_v<T, SolidColor>) {
          // Solid color - convert to variant
          return createPaintFromColor(std::make_optional<Color>(fillValue),
                                      offsetX, offsetY, width, height);
        } else if constexpr (std::is_same_v<T, GradientColor>) {
          // Gradient color - convert to variant
          return createPaintFromColor(std::make_optional<Color>(fillValue),
                                      offsetX, offsetY, width, height);
        } else if constexpr (std::is_same_v<T, ImageFill>) {
          // Image fill
          SkPaint imagePaint;
          imagePaint.setAntiAlias(true);

          // Use the pre-loaded image (loaded during parsing)
          sk_sp<SkImage> image = fillValue.image;
          if (image) {
            // Get image dimensions
            int imageWidth = image->width();
            int imageHeight = image->height();

            if (imageWidth > 0 && imageHeight > 0) {
              if (fillValue.fit == "repeat") {
                SkMatrix localMatrix;
                // Apply rotation around image center if specified
                if (fillValue.rotateBeforeRepeat.has_value() &&
                    *fillValue.rotateBeforeRepeat != 0.0F) {
                  float centerX = static_cast<float>(imageWidth) / 2.0F;
                  float centerY = static_cast<float>(imageHeight) / 2.0F;
                  localMatrix = SkMatrix::Translate(centerX, centerY);
                  localMatrix.postRotate(*fillValue.rotateBeforeRepeat);
                  localMatrix.postTranslate(-centerX, -centerY);
                }
                if (fillValue.scaleBeforeRepeat.has_value()) {
                  localMatrix.postScale(*fillValue.scaleBeforeRepeat,
                                        *fillValue.scaleBeforeRepeat);
                }
                localMatrix.postTranslate(offsetX, offsetY);
                sk_sp<SkShader> imageShader =
                    image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                      SkSamplingOptions(SkFilterMode::kLinear,
                                                        SkMipmapMode::kLinear),
                                      &localMatrix);

                if (imageShader) {
                  // Apply color filters if any are specified
                  sk_sp<SkColorFilter> colorFilter =
                      createColorFilterFromImageFill(fillValue);
                  if (colorFilter) {
                    imageShader = imageShader->makeWithColorFilter(colorFilter);
                  }

                  imagePaint.setShader(imageShader);
                } else {
                  // Fallback to transparent if shader creation fails
                  imagePaint.setColor(SK_ColorTRANSPARENT);
                }
                return imagePaint;
              }

              float scaleX = width / static_cast<float>(imageWidth);
              float scaleY = height / static_cast<float>(imageHeight);
              float translateX = offsetX;
              float translateY = offsetY;

              if (fillValue.fit == "contain") {
                // keep aspect ratio, fit the image into the bounding box
                // Use the smaller scale to ensure the entire image fits
                float scale = std::min(scaleX, scaleY);
                scaleX = scale;
                scaleY = scale;
                // Calculate scaled image dimensions
                float scaledWidth = static_cast<float>(imageWidth) * scale;
                float scaledHeight = static_cast<float>(imageHeight) * scale;
                // Center the image within the bounds
                translateX = offsetX + (width - scaledWidth) / 2.0F;
                translateY = offsetY + (height - scaledHeight) / 2.0F;
              } else if (fillValue.fit == "cover") {
                // Keep aspect ratio, cover the bounding box, cropping if
                // necessary. Use the larger scale to ensure coverage
                float scale = std::max(scaleX, scaleY);
                scaleX = scale;
                scaleY = scale;
                // Calculate scaled image dimensions
                float scaledWidth = static_cast<float>(imageWidth) * scale;
                float scaledHeight = static_cast<float>(imageHeight) * scale;
                // Center the image within the bounds
                translateX = offsetX + (width - scaledWidth) / 2.0F;
                translateY = offsetY + (height - scaledHeight) / 2.0F;
              } else if (fillValue.fit == "fill") {
                // default: Scale X and Y individually to fill the bounding box
              }

              // Create local matrix that maps from shader coordinates to canvas
              // coordinates
              SkMatrix localMatrix;
              localMatrix.postScale(scaleX,
                                    scaleY); // scale the image from (0, 0)
              localMatrix.postTranslate(
                  translateX, translateY); // translate the image to bounds
              sk_sp<SkShader> imageShader =
                  image->makeShader(SkTileMode::kDecal, SkTileMode::kDecal,
                                    SkSamplingOptions(SkFilterMode::kLinear,
                                                      SkMipmapMode::kLinear),
                                    &localMatrix);

              if (imageShader) {
                // Apply color filters if any are specified
                sk_sp<SkColorFilter> colorFilter =
                    createColorFilterFromImageFill(fillValue);
                if (colorFilter) {
                  imageShader = imageShader->makeWithColorFilter(colorFilter);
                }

                imagePaint.setShader(imageShader);
              } else {
                // Fallback to transparent if shader creation fails
                imagePaint.setColor(SK_ColorTRANSPARENT);
              }
            } else {
              // Invalid image dimensions, fallback to transparent
              imagePaint.setColor(SK_ColorTRANSPARENT);
            }
          } else {
            // Failed to load image, fallback to transparent
            imagePaint.setColor(SK_ColorTRANSPARENT);
          }

          return imagePaint;
        }
      },
      fill);
}

// Helper function to load image from data URL with caching
sk_sp<SkImage> loadImageFromData(const std::string &imageData) {
  if (imageData.empty()) {
    return nullptr;
  }

  // Check cache first
  auto &cache = getImageCache();
  auto it = cache.find(imageData);
  if (it != cache.end()) {
    return it->second;
  }

  // Only support data URI (data:image/png;base64,...)
  if (imageData.length() < 5 || imageData.substr(0, 5) != "data:") {
    std::cerr << "Only data URIs are supported, got: "
              << (imageData.length() > 20 ? imageData.substr(0, 20) + "..."
                                          : imageData)
              << std::endl;
    return nullptr;
  }

  size_t commaPos = imageData.find(',');
  if (commaPos == std::string::npos) {
    std::cerr << "Invalid data URI format - no comma found" << std::endl;
    return nullptr;
  }

  // Check bounds to prevent out-of-bounds access
  if (commaPos >= imageData.length() - 1) {
    std::cerr << "Invalid data URI format - no data after comma" << std::endl;
    return nullptr;
  }

  // Extract the base64 data part using string_view to avoid copying
  std::string_view imageDataView = imageData;
  std::string_view base64DataView = imageDataView.substr(commaPos + 1);

  // First call to get the required buffer size
  size_t dataLength = 0;
  SkBase64::Error error = SkBase64::Decode(
      base64DataView.data(), base64DataView.size(), nullptr, &dataLength);
  if (error != SkBase64::kNoError) {
    std::cerr << "Failed to decode base64 data - error: "
              << static_cast<int>(error) << std::endl;
    return nullptr;
  }

  // Validate the decoded data length
  if (dataLength == 0) {
    std::cerr << "Base64 data decodes to zero length" << std::endl;
    return nullptr;
  }

  // Allocate SkData buffer directly (no copy needed)
  sk_sp<SkData> data = SkData::MakeUninitialized(dataLength);
  if (!data) {
    std::cerr << "Failed to allocate SkData buffer" << std::endl;
    return nullptr;
  }

  // Decode directly into the SkData buffer
  error = SkBase64::Decode(base64DataView.data(), base64DataView.size(),
                           data->writable_data(), &dataLength);
  if (error != SkBase64::kNoError) {
    std::cerr << "Failed to decode base64 data - error: "
              << static_cast<int>(error) << std::endl;
    return nullptr;
  }

  // Try to decode the image
  sk_sp<SkImage> image = SkImages::DeferredFromEncodedData(data);
  if (!image) {
    std::cerr << "Failed to decode image from data URL" << std::endl;
    return nullptr;
  }

  // Cache the successfully loaded image
  cache[imageData] = image;

  return image;
}

// Cache management functions
void clearImageCache() { getImageCache().clear(); }

// Helper function to create ImageFilter from effects
sk_sp<SkImageFilter>
createImageFilterFromEffects(const std::vector<Effect> &effects, float offsetX,
                             float offsetY, float width, float height,
                             const std::array<float, 4> &cornerRadii) {
  sk_sp<SkImageFilter> finalFilter = nullptr;

  for (const auto &effect : effects) {
    if (std::holds_alternative<Blur>(effect)) {
      auto blur = std::get<Blur>(effect);
      finalFilter = SkImageFilters::Blur(blur.blur, blur.blur,
                                         SkTileMode::kClamp, finalFilter);
    } else if (std::holds_alternative<DisplacementMap>(effect)) {
      auto displacementMap = std::get<DisplacementMap>(effect);
      sk_sp<SkShader> noiseShader = SkShaders::MakeFractalNoise(
          displacementMap.freqX, displacementMap.freqY, displacementMap.octaves,
          0.0F, nullptr);

      sk_sp<SkImageFilter> noiseFilter =
          SkImageFilters::Shader(std::move(noiseShader));
      finalFilter = SkImageFilters::DisplacementMap(
          SkColorChannel::kR, SkColorChannel::kG, displacementMap.amplitude,
          std::move(noiseFilter), finalFilter);
    } else if (std::holds_alternative<CRTEffect>(effect)) {
      auto crtEffect = std::get<CRTEffect>(effect);

      // Get cached compiled shader effect
      sk_sp<SkRuntimeEffect> crtShaderEffect = getCRTShaderEffect();
      if (!crtShaderEffect) {
        // Continue with previous filter if compilation failed
        continue;
      }

      // Create builder and set uniforms
      SkRuntimeShaderBuilder builder(crtShaderEffect);
      builder.uniform("verticalBleed") = crtEffect.verticalBleed;
      builder.uniform("horizontalBleed") = crtEffect.horizontalBleed;
      builder.uniform("contrast") = crtEffect.contrast;
      builder.uniform("granularity") = crtEffect.granularity;
      // Use the actual image/layer dimensions for size
      builder.uniform("shaderSize") = SkV2{width, height};
      // Pass the offset to adjust coordinates
      builder.uniform("shaderOffset") = SkV2{offsetX, offsetY};

      // Create the runtime shader image filter
      // Use a sample radius to account for the Gaussian filtering
      sk_sp<SkImageFilter> crtFilter =
          SkImageFilters::RuntimeShader(builder, "child", finalFilter);

      finalFilter = crtFilter;
    } else if (std::holds_alternative<LiquidGlass>(effect)) {
      auto liquidGlass = std::get<LiquidGlass>(effect);

      // Get cached compiled shader effect
      sk_sp<SkRuntimeEffect> liquidGlassShaderEffect =
          getLiquidGlassShaderEffect();
      if (!liquidGlassShaderEffect) {
        // Continue with previous filter if compilation failed
        continue;
      }

      // Pass parameters directly to shader
      // Create builder and set uniforms
      SkRuntimeShaderBuilder builder(liquidGlassShaderEffect);
      builder.uniform("rim") = liquidGlass.rim;
      builder.uniform("refraction") = liquidGlass.refraction;
      builder.uniform("tint") = liquidGlass.tint;
      builder.uniform("cornerRadii") =
          SkV4{cornerRadii[0], cornerRadii[1], cornerRadii[2], cornerRadii[3]};
      builder.uniform("shaderSize") = SkV2{width, height};
      builder.uniform("shaderOffset") = SkV2{offsetX, offsetY};

      // Create the runtime shader image filter
      sk_sp<SkImageFilter> liquidGlassFilter =
          SkImageFilters::RuntimeShader(builder, "child", finalFilter);

      finalFilter = liquidGlassFilter;
    }
  }

  return finalFilter;
}

// Helper function to check if clip is enabled
bool isClipEnabled(const Clip &clip) {
  return std::visit([](const auto &clipType) { return clipType.enable; }, clip);
}

// Helper function to check if an effect should be disabled
bool isEffectDisabled(const Effect &effect, const Clip &clip, bool isBackdrop) {
  // Check if clip is enabled
  bool clipEnabled = isClipEnabled(clip);

  // Check if clip is a path
  bool isPathClip = std::holds_alternative<PathClip>(clip);

  // Check effect type
  bool isLiquidGlass = std::holds_alternative<LiquidGlass>(effect);

  if (isBackdrop) {
    // For backdrop effects: clip must be enabled
    if (!clipEnabled) {
      return true;
    }
    // LiquidGlass has additional requirement: clip must not be a path
    if (isLiquidGlass && isPathClip) {
      return true;
    }
  } else {
    // For foreground effects: clip can be disabled except for liquidGlass
    if (isLiquidGlass && !clipEnabled) {
      return true;
    }
  }
  return false;
}

// Helper function to filter effects based on clip state
std::vector<Effect> filterEffects(const std::vector<Effect> &effects,
                                  const Clip &clip, bool isBackdrop) {
  std::vector<Effect> filteredEffects;
  filteredEffects.reserve(effects.size());

  for (const auto &effect : effects) {
    if (!isEffectDisabled(effect, clip, isBackdrop)) {
      filteredEffects.push_back(effect);
    }
  }

  return filteredEffects;
}

// Helper function to adjust bounds for stroke alignment
SkRect adjustBoundsForStroke(const SkRect &bounds, StrokeAlign strokeAlign,
                             float strokeWidth) {
  SkRect adjustedBounds = bounds;
  float halfStroke = strokeWidth / 2.0F;
  bool adjustForStroke = (strokeAlign == StrokeAlign::INSIDE ||
                          strokeAlign == StrokeAlign::OUTSIDE) &&
                         strokeWidth > 0.0F;

  if (adjustForStroke) {
    if (strokeAlign == StrokeAlign::INSIDE) {
      adjustedBounds.inset(halfStroke, halfStroke);
    } else if (strokeAlign == StrokeAlign::OUTSIDE) {
      adjustedBounds.outset(halfStroke, halfStroke);
    }
  }

  return adjustedBounds;
}

// Helper function to extract corner radii from clipPath (for rrect)
// Returns -1 for all corners if clipPath is an oval (no corner radii)
std::array<float, 4> getCornerRadiiFromClip(const SkPath &clipPath) {
  // Check if path is an oval first
  SkRect ovalBounds;
  if (clipPath.isOval(&ovalBounds)) {
    // Oval has no corner radii, return -1 to indicate this
    return {-1.0F, -1.0F, -1.0F, -1.0F};
  }

  // Check if path is an RRect
  SkRRect rrect;
  if (clipPath.isRRect(&rrect)) {
    // Extract effective radii from RRect (these are clamped to valid values)
    return {static_cast<float>(rrect.radii(SkRRect::kUpperLeft_Corner).fX),
            static_cast<float>(rrect.radii(SkRRect::kUpperRight_Corner).fX),
            static_cast<float>(rrect.radii(SkRRect::kLowerRight_Corner).fX),
            static_cast<float>(rrect.radii(SkRRect::kLowerLeft_Corner).fX)};
  }

  // Not an oval or RRect, return zeros
  return {0.0F, 0.0F, 0.0F, 0.0F};
}

// Helper function to get clip path based on clip type
SkPath getClipPath(const Clip &clip, const SkRect &bounds) {
  SkPath clipPath;

  std::visit(
      [&](const auto &clipType) {
        if (clipType.enable) {
          if constexpr (std::is_same_v<std::decay_t<decltype(clipType)>,
                                       RRectClip>) {
            // rrect clip - rounded rectangle
            // Check if any radius is greater than 0
            bool hasRounding =
                std::any_of(clipType.r.begin(), clipType.r.end(),
                            [](float radius) { return radius > 0.0F; });
            if (hasRounding) {
              SkRRect rrect;
              std::array<SkVector, 4> radii = {
                  SkVector::Make(clipType.r[0], clipType.r[0]),
                  SkVector::Make(clipType.r[1], clipType.r[1]),
                  SkVector::Make(clipType.r[2], clipType.r[2]),
                  SkVector::Make(clipType.r[3], clipType.r[3])};
              rrect.setRectRadii(bounds, radii.data());
              // start from top left radius curve
              clipPath = SkPath::RRect(rrect, SkPathDirection::kCW, -1);
            } else {
              // Regular rectangle
              clipPath.addRect(bounds, SkPathDirection::kCW,
                               0); // start from top left corner
            }
          } else if constexpr (std::is_same_v<std::decay_t<decltype(clipType)>,
                                              OvalClip>) {
            // oval clip - ellipse/oval
            clipPath.addOval(bounds, SkPathDirection::kCW,
                             3); // start from left most point
          } else if constexpr (std::is_same_v<std::decay_t<decltype(clipType)>,
                                              PathClip>) {
            // path clip - custom path
            // Path coordinates are stored in node-relative coordinates (0,0 at
            // node's top-left) Translate the path to canvas coordinates by
            // adding the node's position
            clipPath = clipType.path;
            clipPath.offset(bounds.left(), bounds.top());
          }
        } else {
          // No clip, use rectangle
          clipPath.addRect(bounds, SkPathDirection::kCW, 0);
        }
      },
      clip);

  return clipPath;
}

void renderParagraphCursor(skia::textlayout::Paragraph *paragraph,
                           size_t cursorOffset, float x, float y,
                           SkCanvas *canvas) {
  if (!paragraph || !canvas) {
    return;
  }

  // Helper function to render cursor line
  auto renderCursor = [canvas](float cursorX, float cursorY,
                               float cursorHeight) {
    SkPaint cursorPaint;
    cursorPaint.setColor(SK_ColorBLACK);
    cursorPaint.setStyle(SkPaint::kStroke_Style);
    cursorPaint.setStrokeWidth(2.0F);
    cursorPaint.setAntiAlias(true);
    canvas->drawLine(cursorX, cursorY, cursorX, cursorY + cursorHeight,
                     cursorPaint);
  };

  // Cursor offset is at the end of text or out of bounds
  // Try to get position using getRectsForRange
  std::vector<skia::textlayout::TextBox> boxes;

  // If cursorOffset is 0, try to get position at the start
  if (cursorOffset == 0) {
    boxes = paragraph->getRectsForRange(
        0, 1, skia::textlayout::RectHeightStyle::kTight,
        skia::textlayout::RectWidthStyle::kTight);

    if (!boxes.empty()) {
      SkRect firstBox = boxes[0].rect;
      float cursorX = 0;
      if (boxes[0].direction == skia::textlayout::TextDirection::kRtl) {
        cursorX = x + firstBox.fRight;
      } else {
        cursorX = x + firstBox.fLeft;
      }

      float cursorY = y + firstBox.fTop;
      float cursorHeight = firstBox.height();

      renderCursor(cursorX, cursorY, cursorHeight);
    }
  } else {
    // Get position at cursorOffset - 1 to cursorOffset (the character before
    // cursor)
    boxes =
        paragraph->getRectsForRange(static_cast<unsigned>(cursorOffset - 1),
                                    static_cast<unsigned>(cursorOffset),
                                    skia::textlayout::RectHeightStyle::kTight,
                                    skia::textlayout::RectWidthStyle::kTight);

    // Try to get position at cursorOffset itself to check if we're after a
    // newline Try cursorOffset to cursorOffset+1 first, then cursorOffset to
    // cursorOffset as fallback
    std::vector<skia::textlayout::TextBox> boxesAtOffset =
        paragraph->getRectsForRange(static_cast<unsigned>(cursorOffset),
                                    static_cast<unsigned>(cursorOffset + 1),
                                    skia::textlayout::RectHeightStyle::kTight,
                                    skia::textlayout::RectWidthStyle::kTight);

    // If that didn't work, try cursorOffset to cursorOffset (same position)
    if (boxesAtOffset.empty()) {
      boxesAtOffset =
          paragraph->getRectsForRange(static_cast<unsigned>(cursorOffset),
                                      static_cast<unsigned>(cursorOffset),
                                      skia::textlayout::RectHeightStyle::kTight,
                                      skia::textlayout::RectWidthStyle::kTight);
    }

    // If we have valid positions at both cursorOffset-1 and cursorOffset,
    // compare Y coordinates to detect if we're after a newline
    if (!boxesAtOffset.empty() && !boxes.empty()) {
      float yAtOffset = boxesAtOffset[0].rect.fTop;
      float yBeforeOffset = boxes[0].rect.fTop;

      // If Y coordinates are different (with some tolerance for floating
      // point), it means cursorOffset is after a newline and on a new line
      if (std::abs(yAtOffset - yBeforeOffset) > 0.1F) {
        // Use position at cursorOffset (start of next line)
        SkRect offsetBox = boxesAtOffset[0].rect;
        float cursorX = 0;
        if (boxesAtOffset[0].direction ==
            skia::textlayout::TextDirection::kRtl) {
          cursorX = x + offsetBox.fRight;
        } else {
          cursorX = x + offsetBox.fLeft;
        }

        float cursorY = y + offsetBox.fTop;
        float cursorHeight = offsetBox.height();

        renderCursor(cursorX, cursorY, cursorHeight);
        return;
      }
    }

    // Otherwise, use the standard logic: place cursor after cursorOffset - 1
    if (!boxes.empty()) {
      SkRect lastBox = boxes[0].rect;
      float cursorX = 0;
      if (boxes[0].direction == skia::textlayout::TextDirection::kRtl) {
        cursorX = x + lastBox.fLeft;
      } else {
        cursorX = x + lastBox.fRight;
      }

      float cursorY = y + lastBox.fTop;
      float cursorHeight = lastBox.height();

      renderCursor(cursorX, cursorY, cursorHeight);
    }
  }
}

// Helper function to find a node by ID recursively
std::shared_ptr<AnyNode> findNodeById(const std::shared_ptr<AnyNode>& root, int id) {
  if (!root) {
    return nullptr;
  }

  // Check if this node matches
  if (root->id == id) {
    return root;
  }

  // Recursively search children based on node type
  if (root->type == NodeType::LAYER) {
    auto layerNode = std::dynamic_pointer_cast<LayerNode>(root);
    if (layerNode) {
      for (const auto& child : layerNode->children) {
        auto found = findNodeById(child, id);
        if (found) {
          return found;
        }
      }
    }
  } else if (root->type == NodeType::SCREEN) {
    auto screenNode = std::dynamic_pointer_cast<ScreenNode>(root);
    if (screenNode) {
      for (const auto& child : screenNode->children) {
        auto found = findNodeById(child, id);
        if (found) {
          return found;
        }
      }
    }
  } else if (root->type == NodeType::ROOT) {
    auto rootNode = std::dynamic_pointer_cast<RootNode>(root);
    if (rootNode) {
      for (const auto& child : rootNode->children) {
        auto found = findNodeById(child, id);
        if (found) {
          return found;
        }
      }
    }
  }

  return nullptr;
}
} // namespace common_renderer
