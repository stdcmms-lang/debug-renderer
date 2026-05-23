#include "common_renderer.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkStream.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/effects/SkTrimPathEffect.h"
#include "include/encode/SkPngEncoder.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "native_parser.h"
#include "render_utils.h"
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

namespace common_renderer {

// FontManagerSingleton implementation
FontManagerSingleton &FontManagerSingleton::getInstance() {
  static FontManagerSingleton instance;
  return instance;
}

sk_sp<SkFontMgr> FontManagerSingleton::getFontManager() {
  if (!initialized) {
    return nullptr;
  }
  return fontManager;
}

void FontManagerSingleton::setFontManager(sk_sp<SkFontMgr> fontMgr) {
  fontManager = std::move(fontMgr);
  initialized = true;
}

void FontManagerSingleton::reset() {
  fontManager = nullptr;
  initialized = false;
}

/**
 * @brief Converts section-based cursor offset to UTF-16 character offset
 * 
 * The cursor offset system uses a section-based approach where each section
 * counts as 1 offset in addition to its text length. This function converts
 * that to the actual UTF-16 character offset in the concatenated text.
 * 
 * cursor offset is the index of the character BEHIND the cursor,
 * and each section counts as 1 offset. Returns the UTF-16 character offset
 * in the concatenated text, or 0 if cursor is at the start.
 * 
 * @param paragraphNodePtr The paragraph node with text sections
 * @param sectionBasedOffset The section-based cursor offset
 * @return The UTF-16 character offset in the concatenated text
 */
static size_t convertSectionBasedOffsetToUtf16(
    const std::shared_ptr<ParagraphNode> &paragraphNodePtr,
    int sectionBasedOffset) {
  if (sectionBasedOffset <= 0 || paragraphNodePtr->texts.empty()) {
    return 0;
  }

  // Find which section contains cursorOffset (the character after cursor)
  int currentOffset = 0;
  int targetSectionIndex = 0;
  int targetSectionOffset = 0;
  bool found = false;

  for (size_t i = 0; i < paragraphNodePtr->texts.size(); i++) {
    const auto &section = paragraphNodePtr->texts[i];
    // Empty sections count as 1 offset
    int sectionLength = static_cast<int>(section.txt.length()) + 1;
    int nextOffset = currentOffset + sectionLength;

    // Check if cursorOffset falls within this section
    if (sectionBasedOffset >= currentOffset &&
        sectionBasedOffset < nextOffset) {
      targetSectionIndex = static_cast<int>(i);
      targetSectionOffset = sectionBasedOffset - currentOffset;
      found = true;
      break;
    }
    currentOffset = nextOffset;
  }

  if (!found) {
    // Cursor is beyond all sections, return the total text length
    size_t totalLength = 0;
    for (const auto &section : paragraphNodePtr->texts) {
      totalLength += section.txt.length();
    }
    return totalLength;
  }

  // Calculate the actual UTF-16 offset: sum of all previous sections' txt.length
  // + character position in current section
  size_t utf16Offset = 0;
  for (int i = 0; i < targetSectionIndex; i++) {
    utf16Offset += paragraphNodePtr->texts[i].txt.length();
  }
  utf16Offset += static_cast<size_t>(targetSectionOffset);

  return utf16Offset;
}

// Helper function to render a paragraph
bool renderParagraph(SkCanvas *canvas,
                     const std::shared_ptr<ParagraphNode> &paragraphNodePtr,
                     float offsetX, float offsetY, float parentWidth,
                     float parentHeight) {
  if (!canvas || !paragraphNodePtr) {
    return false;
  }
  auto fontMgr = FontManagerSingleton::getInstance().getFontManager();
  if (!fontMgr) {
    std::cerr << "Default font manager is not available" << std::endl;
    return false;
  }

  // Calculate paragraph bounds
  std::vector<float> coords =
      toTlwhPriorRotation(paragraphNodePtr, parentWidth, parentHeight);
  float x = coords[0] + offsetX, y = coords[1] + offsetY, w = coords[2],
        h = coords[3];

  // Create font collection with font manager
  auto fontCollection = sk_make_sp<skia::textlayout::FontCollection>();
  fontCollection->setDefaultFontManager(fontMgr);

  // Set up paragraph style
  skia::textlayout::ParagraphStyle paragraphStyle;
  paragraphStyle.setTextAlign(paragraphNodePtr->textAlign);
  if (!paragraphNodePtr->textOverflow) {
    paragraphStyle.setEllipsis(u"...");
  }

  // Create paragraph builder
  auto paragraphBuilder =
      skia::textlayout::ParagraphBuilder::make(paragraphStyle, fontCollection);

  // Add text sections
  for (const auto &textSection : paragraphNodePtr->texts) {
    // Create text style for this section
    skia::textlayout::TextStyle textStyle;

    // Set font family and size
    if (!textSection.fontFamily.empty()) {
      textStyle.setFontFamilies({SkString(textSection.fontFamily.c_str())});
    }
    textStyle.setFontSize(textSection.fontSize);

    // Set font style (weight and slant)
    SkFontStyle fontStyle(textSection.fontStyle.weight,
                          textSection.fontStyle.weight / 100,
                          textSection.fontStyle.slant);
    textStyle.setFontStyle(fontStyle);

    // Set text color
    if (textSection.foregroundColor) {
      SkPaint paint =
          createPaintFromColor(textSection.foregroundColor, x, y, w, h);
      textStyle.setForegroundPaint(paint);
    }

    // Set letter spacing
    if (textSection.letterSpacing != 0.0F) {
      textStyle.setLetterSpacing(textSection.letterSpacing);
    }

    // Set word spacing
    if (textSection.wordSpacing != 0.0F) {
      textStyle.setWordSpacing(textSection.wordSpacing);
    }

    // Set line height
    if (textSection.lineHeight != 1.0F) {
      textStyle.setHeight(textSection.lineHeight);
      textStyle.setHeightOverride(true);
    }

    textStyle.setFontHinting(SkFontHinting::kSlight);
    textStyle.setFontEdging(SkFont::Edging::kAntiAlias);

    // Set text decoration
    if (textSection.decoration.has_value()) {
      const auto &decoration = textSection.decoration.value();

      if (decoration.type != skia::textlayout::TextDecoration::kNoDecoration) {
        textStyle.setDecoration(decoration.type);
        textStyle.setDecorationStyle(decoration.style);

        // Set decoration thickness
        if (decoration.thickness != 1.0F) {
          textStyle.setDecorationThicknessMultiplier(decoration.thickness);
        }

        // Set decoration color
        if (!decoration.color.empty()) {
          // Parse color string and set decoration color
          SkColor decorationColor = parseColor(decoration.color);
          textStyle.setDecorationColor(decorationColor);
        }
      }
    }

    // Push style and add text
    paragraphBuilder->pushStyle(textStyle);
    paragraphBuilder->addText(textSection.txt.c_str(),
                              textSection.txt.length());
    paragraphBuilder->pop();
  }

  // Build the paragraph
  auto paragraph = paragraphBuilder->Build();

  // Layout the paragraph with the available width
  paragraph->layout(w);

  // Get the actual paragraph height and update h
  if (paragraphNodePtr->alignV == VerticalAlign::CENTER ||
      paragraphNodePtr->alignV == VerticalAlign::BOTTOM) {
    auto actualHeight = paragraph->getHeight();
    paragraphNodePtr->height =
        common_renderer::PositionValue{actualHeight, false};
    std::vector<float> coords =
        toTlwhPriorRotation(paragraphNodePtr, parentWidth, parentHeight);
    x = coords[0];
    y = coords[1];
    w = coords[2];
    h = coords[3];
  }

  // Apply rotation if specified
  if (paragraphNodePtr->rotation.z != 0.0F ||
      paragraphNodePtr->rotation.skewX != 0.0F) {
    // cx and cy are percentages relative to width/height
    float centerX = x + paragraphNodePtr->rotation.cx * w;
    float centerY = y + paragraphNodePtr->rotation.cy * h;

    // Save canvas state
    canvas->save();

    // Translate to rotation center, rotate, then translate back
    canvas->translate(centerX, centerY);
    if (paragraphNodePtr->rotation.z != 0.0F) {
      canvas->rotate(paragraphNodePtr->rotation.z);
    }
    if (paragraphNodePtr->rotation.skewX != 0.0F) {
      canvas->skew(paragraphNodePtr->rotation.skewX, 0.0F);
    }
    canvas->translate(-centerX, -centerY);

    // Paint the paragraph
    paragraph->paint(canvas, x, y);

    // Render cursor if cursorOffset is set (within the same transformation)
    if (paragraphNodePtr->cursorOffset.has_value()) {
      // Convert section-based offset to UTF-16 character offset
      auto utf16Offset = convertSectionBasedOffsetToUtf16(
          paragraphNodePtr, paragraphNodePtr->cursorOffset.value());
      renderParagraphCursor(paragraph.get(), utf16Offset, x, y, canvas);
    }

    // Restore canvas state
    canvas->restore();
  } else {
    // Paint the paragraph directly
    paragraph->paint(canvas, x, y);

    // Render cursor if cursorOffset is set
    if (paragraphNodePtr->cursorOffset.has_value()) {
      // Convert section-based offset to UTF-16 character offset
      auto utf16Offset = convertSectionBasedOffsetToUtf16(
          paragraphNodePtr, paragraphNodePtr->cursorOffset.value());
      renderParagraphCursor(paragraph.get(), utf16Offset, x, y, canvas);
    }
  }

  return true;
}

// Helper function to render a layer
bool renderLayer(SkCanvas *canvas,
                 const std::shared_ptr<LayerNode> &layerNodePtr, float offsetX,
                 float offsetY, float parentWidth, float parentHeight) {
  if (!canvas || !layerNodePtr) {
    return false;
  }

  // Calculate layer bounds
  std::vector<float> coords =
      toTlwhPriorRotation(layerNodePtr, parentWidth, parentHeight);
  float x = coords[0] + offsetX, y = coords[1] + offsetY, w = coords[2],
        h = coords[3];

  // Create bounds for clipping
  SkRect bounds = SkRect::MakeXYWH(x, y, w, h);
  // Adjust bounds for stroke alignment before getting clip path
  if (layerNodePtr->stroke.has_value()) {
    bounds = adjustBoundsForStroke(bounds, layerNodePtr->stroke->align,
                                   layerNodePtr->stroke->width);
  }

  canvas->save();

  // Apply rotation and skew around center point if specified
  if (layerNodePtr->rotation.z != 0.0F ||
      layerNodePtr->rotation.skewX != 0.0F) {
    // cx and cy are percentages relative to width/height
    float centerX = x + layerNodePtr->rotation.cx * w;
    float centerY = y + layerNodePtr->rotation.cy * h;

    // Translate to rotation center, rotate, then translate back
    canvas->translate(centerX, centerY);
    if (layerNodePtr->rotation.z != 0.0F) {
      canvas->rotate(layerNodePtr->rotation.z);
    }
    if (layerNodePtr->rotation.skewX != 0.0F) {
      canvas->skew(layerNodePtr->rotation.skewX, 0.0F);
    }
    canvas->translate(-centerX, -centerY);
  }

  // Get the clip path for this layer (clip.enable = false by default)
  const Clip &clip = layerNodePtr->clip.has_value() ? layerNodePtr->clip.value()
                                                    : Clip{RRectClip{}};
  SkPath clipPath = getClipPath(clip, bounds);
  bool clipEnabled = isClipEnabled(clip);

  // Draw fill first (behind children)
  if (layerNodePtr->fill.has_value()) {
    SkPaint fillPaint =
        createPaintFromFill(layerNodePtr->fill.value(), x, y, w, h);
    fillPaint.setStyle(SkPaint::kFill_Style);
    // Draw using the clip path shape
    canvas->drawPath(clipPath, fillPaint);
  }

  if (clipEnabled) {
    canvas->save();
    // Apply clip without trimming
    canvas->clipPath(clipPath, true);
  }

  // Extract corner radii from clipPath for LiquidGlass effect
  // This gets effective radii (clamped if user set values too large)
  std::array<float, 4> cornerRadii = getCornerRadiiFromClip(clipPath);

  // Backdrop effects - filter disabled effects before applying
  std::vector<Effect> filteredBackdropEffects =
      filterEffects(layerNodePtr->backdropEffects, clip, true);
  if (!filteredBackdropEffects.empty()) {
    sk_sp<SkImageFilter> finalFilter = createImageFilterFromEffects(
        filteredBackdropEffects, x, y, w, h, cornerRadii);
    SkCanvas::SaveLayerRec rec(&bounds, nullptr, finalFilter.get(), 0);
    canvas->saveLayer(rec);
  }

  // Opacity or foreground effects - filter disabled effects before applying
  std::vector<Effect> filteredForegroundEffects =
      filterEffects(layerNodePtr->foregroundEffects, clip, false);
  float opacity = 1.0F;
  if (layerNodePtr->opacity.has_value()) {
    opacity = layerNodePtr->opacity.value();
  }
  if (opacity < 1.0F || !filteredForegroundEffects.empty()) {
    // Create paint for layer effects
    SkPaint layerPaint;
    layerPaint.setAntiAlias(true);

    sk_sp<SkImageFilter> finalFilter = createImageFilterFromEffects(
        filteredForegroundEffects, x, y, w, h, cornerRadii);
    layerPaint.setImageFilter(finalFilter);
    // Save layer with paint for opacity and foreground effects
    canvas->saveLayer(clipEnabled ? &bounds : nullptr, &layerPaint);
  }

  // Render all children
  for (const auto &child : layerNodePtr->children) {
    if (child) {
      drawNode(canvas, child, x, y, w, h);
    }
  }

  if (opacity < 1.0F ||
      !filteredForegroundEffects.empty()) { // Opacity or foreground effects
    canvas->restore();
  }
  if (!filteredBackdropEffects.empty()) { // Backdrop effects
    canvas->restore();
  }
  if (clipEnabled) { // Clip
    canvas->restore();
  }

  if (layerNodePtr->stroke.has_value()) {
    SkPaint strokePaint =
        createPaintFromColor(layerNodePtr->stroke->color, x, y, w, h);
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setStrokeWidth(layerNodePtr->stroke->width);
    // Draw stroke using the clip path
    canvas->drawPath(clipPath, strokePaint);
  }

  canvas->restore();

  return true;
}

// Helper function to render a screen
bool renderScreen(SkCanvas *canvas,
                  const std::shared_ptr<ScreenNode> &screenNodePtr,
                  float offsetX, float offsetY, float parentWidth,
                  float parentHeight) {
  if (!canvas || !screenNodePtr) {
    return false;
  }

  // Convert ScreenNode to LayerNode to reuse render_layer logic
  auto layerNode = std::make_shared<LayerNode>();
  layerNode->name = screenNodePtr->name;
  layerNode->id = screenNodePtr->id;
  layerNode->alignH = screenNodePtr->alignH;
  layerNode->alignV = screenNodePtr->alignV;
  layerNode->left = screenNodePtr->left;
  layerNode->right = screenNodePtr->right;
  layerNode->top = screenNodePtr->top;
  layerNode->bottom = screenNodePtr->bottom;
  layerNode->width = screenNodePtr->width;
  layerNode->height = screenNodePtr->height;

  // Copy children and clip settings
  layerNode->children = screenNodePtr->children;
  layerNode->clip = screenNodePtr->clip;
  layerNode->fill = screenNodePtr->fill;
  layerNode->stroke = screenNodePtr->stroke;

  // Use render_layer to handle all the layer rendering logic
  return renderLayer(canvas, layerNode, offsetX, offsetY, parentWidth,
                     parentHeight);
}

bool renderSvg(SkCanvas *canvas, const std::shared_ptr<SVGNode> &svgNodePtr,
               float offsetX, float offsetY, float parentWidth,
               float parentHeight) {
  if (!canvas || !svgNodePtr) {
    return false;
  }

  // Calculate SVG bounds
  std::vector<float> coords =
      toTlwhPriorRotation(svgNodePtr, parentWidth, parentHeight);
  float x = coords[0] + offsetX, y = coords[1] + offsetY, w = coords[2],
        h = coords[3];

  // Create a memory stream from the SVG string
  auto stream = std::make_unique<SkMemoryStream>(svgNodePtr->svg.data(),
                                                 svgNodePtr->svg.size(), true);

  if (!stream) {
    std::cerr << "Failed to create memory stream for SVG" << std::endl;
    return false;
  }

  // Create SVG DOM from the stream
  auto svgDom = SkSVGDOM::MakeFromStream(*stream);
  if (!svgDom) {
    std::cerr << "Failed to parse SVG" << std::endl;
    return false;
  }

  // Set the container size for the SVG
  svgDom->setContainerSize(SkSize::Make(w, h));

  canvas->save();

  // Translate to the SVG position
  canvas->translate(x, y);

  // Render the SVG
  svgDom->render(canvas);

  canvas->restore();

  return true;
}

// Helper function to render a path
bool renderPath(SkCanvas *canvas, const std::shared_ptr<PathNode> &pathNodePtr,
                float offsetX, float offsetY, float parentWidth,
                float parentHeight) {
  if (!canvas || !pathNodePtr) {
    return false;
  }

  // Calculate path bounds
  std::vector<float> coords =
      toTlwhPriorRotation(pathNodePtr, parentWidth, parentHeight);
  float x = coords[0], y = coords[1], w = coords[2],
        h = coords[3];

  // Get the path from the node
  SkPath path = pathNodePtr->path;
  if (path.isEmpty()) {
    // Empty path, nothing to render
    return true;
  }

  canvas->save();

  // Path is stored in node-relative coordinates (0,0 at node's top-left)
  // First, translate the path to canvas coordinates by adding the node's
  // position
  SkPath translatedPath = path;
  translatedPath.offset(x, y);

  // Apply rotation and skew around center point if specified
  if (pathNodePtr->rotation.z != 0.0F || pathNodePtr->rotation.skewX != 0.0F) {
    // cx and cy are percentages relative to width/height
    float centerX = x + pathNodePtr->rotation.cx * w;
    float centerY = y + pathNodePtr->rotation.cy * h;

    // Translate to rotation center, rotate, then translate back
    canvas->translate(centerX, centerY);
    if (pathNodePtr->rotation.z != 0.0F) {
      canvas->rotate(pathNodePtr->rotation.z);
    }
    if (pathNodePtr->rotation.skewX != 0.0F) {
      canvas->skew(pathNodePtr->rotation.skewX, 0.0F);
    }
    canvas->translate(-centerX, -centerY);
  }

  // Apply trim effect to the path if specified
  SkPath trimmedPath = translatedPath;
  if (pathNodePtr->trim.has_value() &&
      (pathNodePtr->trim->start != 0.0F || pathNodePtr->trim->end != 1.0F) &&
      pathNodePtr->trim->start < pathNodePtr->trim->end) {
    auto trimEffect =
        SkTrimPathEffect::Make(pathNodePtr->trim->start, pathNodePtr->trim->end,
                               SkTrimPathEffect::Mode::kNormal);
    if (trimEffect) {
      SkStrokeRec rec(SkStrokeRec::kHairline_InitStyle);
      SkPathBuilder builder;
      if (trimEffect->filterPath(&builder, translatedPath, &rec, nullptr,
                                 SkMatrix::I())) {
        trimmedPath = builder.detach();
      }
    }
  }

  // Draw fill first
  if (pathNodePtr->fill.has_value()) {
    SkPaint fillPaint =
        createPaintFromFill(pathNodePtr->fill.value(), x, y, w, h);
    fillPaint.setStyle(SkPaint::kFill_Style);
    canvas->drawPath(trimmedPath, fillPaint);
  }

  // Draw stroke second
  if (pathNodePtr->stroke.has_value()) {
    SkPaint strokePaint =
        createPaintFromColor(pathNodePtr->stroke->color, x, y, w, h);
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setStrokeWidth(pathNodePtr->stroke->width);
    canvas->drawPath(trimmedPath, strokePaint);
  }

  canvas->restore();

  return true;
}

// Helper function to render a root node
bool renderRoot(SkCanvas *canvas, const std::shared_ptr<RootNode> &rootNodePtr,
                float offsetX, float offsetY, float parentWidth,
                float parentHeight) {
  if (!canvas || !rootNodePtr) {
    return false;
  }

  canvas->save();
  // Apply transformations in order: origin, scale, pan
  auto [originX, originY] = rootNodePtr->origin;
  auto [scaleX, scaleY] = rootNodePtr->scale;
  auto [panX, panY] = rootNodePtr->pan;

  // before transform: (x,y)
  // after transform: ((x-originX)*scaleX+originX+panX,
  // (y-originY)*scaleY+originY+panY))
  canvas->translate(panX + originX, panY + originY);
  canvas->scale(scaleX, scaleY);
  canvas->translate(-originX, -originY);

  // Render all children with the transformed coordinate system
  for (const auto &child : rootNodePtr->children) {
    if (child) {
      drawNode(canvas, child, offsetX, offsetY, parentWidth, parentHeight);
    }
  }

  canvas->restore();

  return true;
}

bool drawNode(SkCanvas *canvas, const std::shared_ptr<AnyNode> &node,
              float offsetX, float offsetY, float parentWidth,
              float parentHeight) {
  // Route to appropriate renderer based on node type
  switch (node->type) {
  case NodeType::PARAGRAPH: {
    std::shared_ptr<ParagraphNode> paragraphNodePtr =
        std::static_pointer_cast<ParagraphNode>(node);
    return renderParagraph(canvas, paragraphNodePtr, offsetX, offsetY,
                           parentWidth, parentHeight);
  }
  case NodeType::LAYER: {
    std::shared_ptr<LayerNode> layerNodePtr =
        std::static_pointer_cast<LayerNode>(node);
    return renderLayer(canvas, layerNodePtr, offsetX, offsetY, parentWidth,
                       parentHeight);
  }
  case NodeType::SCREEN: {
    std::shared_ptr<ScreenNode> screenNodePtr =
        std::static_pointer_cast<ScreenNode>(node);
    return renderScreen(canvas, screenNodePtr, offsetX, offsetY, parentWidth,
                        parentHeight);
  }
  case NodeType::SVG: {
    std::shared_ptr<SVGNode> svgNodePtr =
        std::static_pointer_cast<SVGNode>(node);
    return renderSvg(canvas, svgNodePtr, offsetX, offsetY, parentWidth,
                     parentHeight);
  }
  case NodeType::PATH: {
    std::shared_ptr<PathNode> pathNodePtr =
        std::static_pointer_cast<PathNode>(node);
    return renderPath(canvas, pathNodePtr, offsetX, offsetY, parentWidth,
                      parentHeight);
  }
  case NodeType::ROOT: {
    std::shared_ptr<RootNode> rootNodePtr =
        std::static_pointer_cast<RootNode>(node);
    return renderRoot(canvas, rootNodePtr, offsetX, offsetY, parentWidth,
                      parentHeight);
  }
  default: {
    std::cerr << "Unknown node type: " << static_cast<int>(node->type)
              << std::endl;
    return false;
  }
  }
}

bool draw(SkCanvas *canvas, const std::shared_ptr<AnyNode> &rootNode,
          float canvasWidth, float canvasHeight, bool clearCanvas) {
  if (!canvas || !rootNode) {
    std::cerr << "Canvas or root node is null" << std::endl;
    return false;
  }

  // Clear the canvas before drawing
  if (clearCanvas) {
    canvas->clear(SK_ColorTRANSPARENT);
  }

  bool success =
      drawNode(canvas, rootNode, 0.0F, 0.0F, canvasWidth, canvasHeight);
  return success;
}

bool drawTransition(SkCanvas *canvas, sk_sp<SkImage> sourceImage,
                    sk_sp<SkImage> targetImage, float progress,
                    TransitionType animationType, float directionX,
                    float directionY, float canvasWidth, float canvasHeight,
                    float dpr) {
  if (!canvas || !sourceImage || !targetImage) {
    std::cerr << "Canvas or images are null" << std::endl;
    return false;
  }

  // Clamp progress to [0.0, 1.0]
  progress = std::max(0.0F, std::min(1.0F, progress));

  // Get transition shader effect based on animation type
  sk_sp<SkRuntimeEffect> transitionEffect = nullptr;
  if (animationType == TransitionType::MOVEIN) {
    transitionEffect = getMoveInShaderEffect();
  } else if (animationType == TransitionType::MOVEOUT) {
    transitionEffect = getMoveOutShaderEffect();
  } else {
    // Fallback to move-in for unknown types
    transitionEffect = getMoveInShaderEffect();
  }

  if (!transitionEffect) {
    std::cerr << "Failed to get transition shader effect" << std::endl;
    return false;
  }

  // Create shader builder
  SkRuntimeShaderBuilder builder(transitionEffect);

  // Create local matrix to scale image shader coordinates by 1/dpr
  // This maps image pixel coordinates to local coordinate space
  SkMatrix imageMatrix;
  imageMatrix.setScale(1.0F / dpr, 1.0F / dpr);

  // Set shader inputs
  builder.child("sourceImage") =
      sourceImage->makeShader(SkSamplingOptions(), &imageMatrix);
  builder.child("targetImage") =
      targetImage->makeShader(SkSamplingOptions(), &imageMatrix);
  builder.uniform("progress") = progress;
  builder.uniform("direction") = SkV2{directionX, directionY};
  builder.uniform("shaderSize") =
      SkV2{static_cast<float>(sourceImage->width()) / dpr,
           static_cast<float>(sourceImage->height()) / dpr};

  // Create shader from builder
  sk_sp<SkShader> shader = builder.makeShader();
  if (!shader) {
    std::cerr << "Failed to create transition shader" << std::endl;
    return false;
  }

  // Create paint with shader
  SkPaint paint;
  paint.setShader(shader);
  paint.setAntiAlias(true);

  // Draw fullscreen rectangle with the transition shader
  SkRect rect = SkRect::MakeWH(canvasWidth, canvasHeight);
  canvas->drawRect(rect, paint);

  return true;
}

size_t makeSnapshot(SkSurface *surface, uint8_t **outPtr) {
  if (!surface || !outPtr) {
    std::cerr << "Surface or out ptr is null" << std::endl;
    return 0;
  }

  // Initialize output pointer to null
  *outPtr = nullptr;

  sk_sp<SkImage> surfaceImage = surface->makeImageSnapshot();
  if (!surfaceImage) {
    std::cerr << "Failed to make surface image snapshot" << std::endl;
    return 0;
  }

  // Convert GPU image to CPU-accessible format for encoding
  // GPU-backed images cannot be directly encoded to PNG
  sk_sp<SkImage> cpuImage = surfaceImage->makeRasterImage();
  if (!cpuImage) {
    std::cerr << "Failed to convert GPU image to raster format" << std::endl;
    return 0;
  }

  // Calculate image size (width * height)
  int64_t imageSize = static_cast<int64_t>(cpuImage->width()) *
                      static_cast<int64_t>(cpuImage->height());

  // Configure PNG compression options based on image size
  // For larger images, use higher compression to reduce file size
  SkPngEncoder::Options pngOptions;

  // Threshold: 800x600 pixels
  const int64_t LARGE_IMAGE_THRESHOLD = 800 * 600;

  if (imageSize > LARGE_IMAGE_THRESHOLD) {
    // For large images, use maximum compression
    pngOptions.fZLibLevel = 9;
    pngOptions.fFilterFlags = SkPngEncoder::FilterFlag::kAll;
  } else {
    // For smaller images, use default compression (balanced speed/size)
    pngOptions.fZLibLevel = 6;
    pngOptions.fFilterFlags = SkPngEncoder::FilterFlag::kAll;
  }

  // Encode to PNG with compression options
  sk_sp<SkData> data =
      SkPngEncoder::Encode(nullptr, cpuImage.get(), pngOptions);
  if (!data || data->size() == 0) {
    std::cerr << "Failed to encode image to PNG" << std::endl;
    return 0;
  }

  // Allocate memory and copy PNG bytes
  size_t size = data->size();
  // Memory allocated here is freed by index.ts draw() in JavaScript/TypeScript
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc,hicpp-no-malloc)
  *outPtr = static_cast<uint8_t *>(std::malloc(size));
  if (!*outPtr) {
    std::cerr << "Failed to allocate memory for PNG bytes" << std::endl;
    return 0;
  }

  std::memcpy(*outPtr, data->data(), size);

  return size;
}

// Helper function to get direction vector from TransitionDirection
SkPoint getDirectionVector(TransitionDirection dir) {
  switch (dir) {
  case TransitionDirection::LEFT:
    return SkPoint::Make(-1.0F, 0.0F);
  case TransitionDirection::RIGHT:
    return SkPoint::Make(1.0F, 0.0F);
  case TransitionDirection::UP:
    return SkPoint::Make(0.0F, -1.0F);
  case TransitionDirection::DOWN:
    return SkPoint::Make(0.0F, 1.0F);
  default:
    return SkPoint::Make(1.0F, 0.0F); // Default to right
  }
}

// Helper function to get current time in milliseconds
int64_t getCurrentTimeMs() {
  // Use std::chrono for platform-agnostic time retrieval
  auto now = std::chrono::steady_clock::now();
  auto duration = now.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(duration)
      .count();
}

// Initialize screen nodes from design state JSON strings
int initializeScreenNodes(
    const std::vector<const char *> &designStateStrings, float logicalWidth,
    float logicalHeight,
    std::map<int, std::shared_ptr<ScreenNode>> &screenNodesMap) {
  // Clear existing map
  screenNodesMap.clear();

  // Iterate over vector of JSON strings and parse each ScreenNode
  for (const char *screenJson : designStateStrings) {
    if (screenJson == nullptr) {
      continue;
    }

    // Use device's logical screen size for root parsing
    auto node = skia_parser::parseNode(screenJson, logicalWidth, logicalHeight);
    if (node) {
      auto screenNode = std::dynamic_pointer_cast<ScreenNode>(node);
      if (screenNode) {
        screenNodesMap[screenNode->id] = screenNode;
      }
    }
  }

  // Return the ID of the first screen, or -1 if no screens were parsed
  if (!screenNodesMap.empty()) {
    return screenNodesMap.begin()->first;
  }
  return -1;
}

} // namespace common_renderer
