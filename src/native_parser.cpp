#include "native_parser.h"
#include "render_utils.h"

#include <cmath>
#include <cstring>
#include <map>

namespace skia_parser {

// Helper function to apply positioning to a node
template <typename T>
void applyPositioning(T *node,
                      const common_renderer::Positioning &positioning) {
  node->alignH = positioning.alignH;
  node->alignV = positioning.alignV;
  node->left = positioning.left;
  node->right = positioning.right;
  node->top = positioning.top;
  node->bottom = positioning.bottom;
  node->width = positioning.width;
  node->height = positioning.height;
}

// Helper function to convert string to HorizontalAlign enum
common_renderer::HorizontalAlign
parseHorizontalAlign(const std::string &alignStr) {
  if (alignStr == "left") {
    return common_renderer::HorizontalAlign::LEFT;
  } else if (alignStr == "center") {
    return common_renderer::HorizontalAlign::CENTER;
  } else if (alignStr == "right") {
    return common_renderer::HorizontalAlign::RIGHT;
  } else if (alignStr == "edge") {
    return common_renderer::HorizontalAlign::EDGE;
  } else {
    // Default to LEFT
    return common_renderer::HorizontalAlign::LEFT;
  }
}

// Helper function to convert string to VerticalAlign enum
common_renderer::VerticalAlign parseVerticalAlign(const std::string &alignStr) {
  if (alignStr == "top") {
    return common_renderer::VerticalAlign::TOP;
  } else if (alignStr == "center") {
    return common_renderer::VerticalAlign::CENTER;
  } else if (alignStr == "bottom") {
    return common_renderer::VerticalAlign::BOTTOM;
  } else if (alignStr == "edge") {
    return common_renderer::VerticalAlign::EDGE;
  } else {
    // Default to TOP
    return common_renderer::VerticalAlign::TOP;
  }
}

// Helper function to convert string to StrokeAlign enum
common_renderer::StrokeAlign parseStrokeAlign(const std::string &alignStr) {
  if (alignStr == "inside") {
    return common_renderer::StrokeAlign::INSIDE;
  } else if (alignStr == "outside") {
    return common_renderer::StrokeAlign::OUTSIDE;
  } else {
    // Default to CENTER
    return common_renderer::StrokeAlign::CENTER;
  }
}

// Helper function to convert string to TextDecoration enum
skia::textlayout::TextDecoration
parseTextDecorationType(const std::string &typeStr) {
  if (typeStr == "none") {
    return skia::textlayout::TextDecoration::kNoDecoration;
  } else if (typeStr == "underline") {
    return skia::textlayout::TextDecoration::kUnderline;
  } else if (typeStr == "overline") {
    return skia::textlayout::TextDecoration::kOverline;
  } else if (typeStr == "line-through") {
    return skia::textlayout::TextDecoration::kLineThrough;
  } else {
    return skia::textlayout::TextDecoration::kNoDecoration;
  }
}

// Helper function to convert string to TextDecorationStyle enum
skia::textlayout::TextDecorationStyle
parseTextDecorationStyle(const std::string &styleStr) {
  if (styleStr == "solid") {
    return skia::textlayout::TextDecorationStyle::kSolid;
  } else if (styleStr == "double") {
    return skia::textlayout::TextDecorationStyle::kDouble;
  } else if (styleStr == "dashed") {
    return skia::textlayout::TextDecorationStyle::kDashed;
  } else if (styleStr == "wavy") {
    return skia::textlayout::TextDecorationStyle::kWavy;
  } else {
    return skia::textlayout::TextDecorationStyle::kSolid;
  }
}

// Helper functions for working with skjson::Value
std::string getString(const skjson::Value &v, const std::string &defaultValue) {
  if (v.getType() == skjson::Value::Type::kNull) {
    return defaultValue;
  }
  if (const skjson::StringValue *str = v) {
    return {str->begin(), str->size()};
  }
  return defaultValue;
}

float getFloat(const skjson::Value &v, float defaultValue) {
  if (v.getType() == skjson::Value::Type::kNull) {
    return defaultValue;
  }
  if (const skjson::NumberValue *num = v) {
    return static_cast<float>(**num);
  }
  return defaultValue;
}

int getInt(const skjson::Value &v, int defaultValue) {
  if (v.getType() == skjson::Value::Type::kNull) {
    return defaultValue;
  }
  if (const skjson::NumberValue *num = v) {
    return static_cast<int>(**num);
  }
  return defaultValue;
}

bool getBool(const skjson::Value &v, bool defaultValue) {
  if (v.getType() == skjson::Value::Type::kNull) {
    return defaultValue;
  }
  if (const skjson::BoolValue *b = v) {
    return **b;
  }
  return defaultValue;
}

bool valHas(const skjson::Value &v, const char *key) {
  if (v.getType() != skjson::Value::Type::kObject) {
    return false;
  }
  const auto &obj = v.as<skjson::ObjectValue>();
  const skjson::Value &val = obj[key];
  return val.getType() != skjson::Value::Type::kNull;
}

// Helper function to parse an image fill from skjson::Value
common_renderer::ImageFill parseImageFill(const skjson::Value &fillObj) {
  common_renderer::ImageFill imageFill;
  std::string imageUrl = getString(fillObj["imageUrl"], "");
  // Load the image immediately during parsing
  imageFill.image = common_renderer::loadImageFromData(imageUrl);
  imageFill.fit = getString(fillObj["fit"], "fill");

  // Parse scaleBeforeRepeat if present
  if (valHas(fillObj, "scaleBeforeRepeat")) {
    imageFill.scaleBeforeRepeat = getFloat(fillObj["scaleBeforeRepeat"], 1.0F);
  }

  // Parse rotation if present
  if (valHas(fillObj, "rotateBeforeRepeat")) {
    imageFill.rotateBeforeRepeat =
        getFloat(fillObj["rotateBeforeRepeat"], 0.0F);
  }

  // Parse filter values if present
  if (valHas(fillObj, "exposure")) {
    imageFill.exposure = getFloat(fillObj["exposure"], 0.0F);
  }
  if (valHas(fillObj, "contrast")) {
    imageFill.contrast = getFloat(fillObj["contrast"], 0.0F);
  }
  if (valHas(fillObj, "saturation")) {
    imageFill.saturation = getFloat(fillObj["saturation"], 0.0F);
  }
  if (valHas(fillObj, "temperature")) {
    imageFill.temperature = getFloat(fillObj["temperature"], 0.0F);
  }
  if (valHas(fillObj, "tint")) {
    imageFill.tint = getFloat(fillObj["tint"], 0.0F);
  }
  if (valHas(fillObj, "highlights")) {
    imageFill.highlights = getFloat(fillObj["highlights"], 0.0F);
  }
  if (valHas(fillObj, "shadows")) {
    imageFill.shadows = getFloat(fillObj["shadows"], 0.0F);
  }

  return imageFill;
}

// Helper function to parse a single shadow from skjson::Value
common_renderer::Shadow parseShadow(const skjson::Value &shadowObj) {
  return common_renderer::Shadow{
      getString(shadowObj["type"], "dropShadow"),
      getFloat(shadowObj["offsetX"], 0.0F),
      getFloat(shadowObj["offsetY"], 0.0F),
      getFloat(shadowObj["blurX"], 0.0F),
      getFloat(shadowObj["blurY"], 0.0F),
      getString(shadowObj["color"], "")};
}

// Helper function to parse shadows (array or single) with backward compatibility
std::vector<common_renderer::Shadow>
parseShadows(const skjson::Value &colorObj) {
  std::vector<common_renderer::Shadow> shadows;

  // Check for "shadows" array first (new format)
  if (valHas(colorObj, "shadows")) {
    const skjson::Value &shadowsArray = colorObj["shadows"];
    if (shadowsArray.is<skjson::ArrayValue>()) {
      const auto &arr = shadowsArray.as<skjson::ArrayValue>();
      for (const auto &shadowValue : arr) {
        shadows.push_back(parseShadow(shadowValue));
      }
    }
  }
  // Backward compatibility: check for single "shadow" object
  else if (valHas(colorObj, "shadow")) {
    const skjson::Value &shadow = colorObj["shadow"];
    shadows.push_back(parseShadow(shadow));
  }

  return shadows;
}

// Helper function to parse a solid color from skjson::Value
common_renderer::SolidColor parseSolidColor(const skjson::Value &colorObj) {
  common_renderer::SolidColor solidColor;
  solidColor.color = getString(colorObj["color"], "");

  // Parse shadows (array or single with backward compatibility)
  solidColor.shadows = parseShadows(colorObj);

  return solidColor;
}

// Helper function to parse a gradient color from skjson::Value
common_renderer::GradientColor
parseGradientColor(const skjson::Value &colorObj) {
  common_renderer::GradientColor gradientColor;
  gradientColor.startColor = getString(colorObj["startColor"], "");
  gradientColor.endColor = getString(colorObj["endColor"], "");

  // Parse start point (mapped from "start" in JSON to "startPoint" in C++)
  if (valHas(colorObj, "start")) {
    const skjson::Value &start = colorObj["start"];
    gradientColor.startPoint = common_renderer::Point{
        getFloat(start["x"], 0.0F), getFloat(start["y"], 0.0F)};
  }

  // Parse end point (mapped from "end" in JSON to "endPoint" in C++)
  if (valHas(colorObj, "end")) {
    const skjson::Value &end = colorObj["end"];
    gradientColor.endPoint = common_renderer::Point{getFloat(end["x"], 1.0F),
                                                    getFloat(end["y"], 1.0F)};
  }

  // Parse shadows (array or single with backward compatibility)
  gradientColor.shadows = parseShadows(colorObj);

  return gradientColor;
}

// Helper function to parse a color from skjson::Value
std::optional<common_renderer::Color>
parseColor(const skjson::Value &colorObj) {
  std::string colorType = getString(colorObj["type"], "solid");

  if (colorType == "solid") {
    auto solidColor = parseSolidColor(colorObj);
    return common_renderer::Color(solidColor);
  } else if (colorType == "gradient") {
    auto gradientColor = parseGradientColor(colorObj);
    return common_renderer::Color(gradientColor);
  }

  // Default to solid color if type is unknown
  auto solidColor = parseSolidColor(colorObj);
  return common_renderer::Color(solidColor);
}

// Helper function to parse a fill from skjson::Value
common_renderer::Fill parseFill(const skjson::Value &fillObj) {
  std::string fillType = getString(fillObj["type"], "solid");

  if (fillType == "solid") {
    return parseSolidColor(fillObj);
  } else if (fillType == "gradient") {
    return parseGradientColor(fillObj);
  } else if (fillType == "image") {
    return parseImageFill(fillObj);
  }

  // Default to solid color if type is unknown
  return parseSolidColor(fillObj);
}

// Helper function to parse rotation from skjson::Value
// cx and cy are percentages relative to width/height, default to 0.5 (center)
common_renderer::Rotation parseRotation(const skjson::Value &rotationObj) {
  return common_renderer::Rotation{
      getFloat(rotationObj["cx"], 0.5F), getFloat(rotationObj["cy"], 0.5F),
      getFloat(rotationObj["z"], 0.0F), getFloat(rotationObj["skewX"], 0.0F)};
}

// Helper function to parse positioning from skjson::Value
common_renderer::Positioning parsePositioning(const skjson::Value &nodeObj) {
  common_renderer::Positioning pos;

  // Parse horizontalAlignment as object with type field
  if (valHas(nodeObj, "horizontalAlignment")) {
    const skjson::Value &hAlign = nodeObj["horizontalAlignment"];
    std::string hAlignType = getString(hAlign["type"], "left");
    pos.alignH = parseHorizontalAlign(hAlignType);

    // Parse fields based on alignment type
    if (hAlignType == "left") {
      if (valHas(hAlign, "left")) {
        float leftValue = getFloat(hAlign["left"], 0.0F);
        bool leftIsPct = getBool(hAlign["leftIsPct"], false);
        pos.left = common_renderer::PositionValue{leftValue, leftIsPct};
      }
      if (valHas(hAlign, "width")) {
        float widthValue = getFloat(hAlign["width"], 0.0F);
        bool widthIsPct = getBool(hAlign["widthIsPct"], false);
        pos.width = common_renderer::PositionValue{widthValue, widthIsPct};
      }
    } else if (hAlignType == "center") {
      if (valHas(hAlign, "width")) {
        float widthValue = getFloat(hAlign["width"], 0.0F);
        bool widthIsPct = getBool(hAlign["widthIsPct"], false);
        pos.width = common_renderer::PositionValue{widthValue, widthIsPct};
      }
    } else if (hAlignType == "right") {
      if (valHas(hAlign, "right")) {
        float rightValue = getFloat(hAlign["right"], 0.0F);
        bool rightIsPct = getBool(hAlign["rightIsPct"], false);
        pos.right = common_renderer::PositionValue{rightValue, rightIsPct};
      }
      if (valHas(hAlign, "width")) {
        float widthValue = getFloat(hAlign["width"], 0.0F);
        bool widthIsPct = getBool(hAlign["widthIsPct"], false);
        pos.width = common_renderer::PositionValue{widthValue, widthIsPct};
      }
    } else if (hAlignType == "edge") {
      if (valHas(hAlign, "left")) {
        float leftValue = getFloat(hAlign["left"], 0.0F);
        bool leftIsPct = getBool(hAlign["leftIsPct"], false);
        pos.left = common_renderer::PositionValue{leftValue, leftIsPct};
      }
      if (valHas(hAlign, "right")) {
        float rightValue = getFloat(hAlign["right"], 0.0F);
        bool rightIsPct = getBool(hAlign["rightIsPct"], false);
        pos.right = common_renderer::PositionValue{rightValue, rightIsPct};
      }
    }
  } else {
    pos.alignH = common_renderer::HorizontalAlign::LEFT;
  }

  // Parse verticalAlignment as object with type field
  if (valHas(nodeObj, "verticalAlignment")) {
    const skjson::Value &vAlign = nodeObj["verticalAlignment"];
    std::string vAlignType = getString(vAlign["type"], "top");
    pos.alignV = parseVerticalAlign(vAlignType);

    // Parse fields based on alignment type
    if (vAlignType == "top") {
      if (valHas(vAlign, "top")) {
        float topValue = getFloat(vAlign["top"], 0.0F);
        bool topIsPct = getBool(vAlign["topIsPct"], false);
        pos.top = common_renderer::PositionValue{topValue, topIsPct};
      }
      if (valHas(vAlign, "height")) {
        float heightValue = getFloat(vAlign["height"], 0.0F);
        bool heightIsPct = getBool(vAlign["heightIsPct"], false);
        pos.height = common_renderer::PositionValue{heightValue, heightIsPct};
      }
    } else if (vAlignType == "center") {
      if (valHas(vAlign, "height")) {
        float heightValue = getFloat(vAlign["height"], 0.0F);
        bool heightIsPct = getBool(vAlign["heightIsPct"], false);
        pos.height = common_renderer::PositionValue{heightValue, heightIsPct};
      }
    } else if (vAlignType == "bottom") {
      if (valHas(vAlign, "bottom")) {
        float bottomValue = getFloat(vAlign["bottom"], 0.0F);
        bool bottomIsPct = getBool(vAlign["bottomIsPct"], false);
        pos.bottom = common_renderer::PositionValue{bottomValue, bottomIsPct};
      }
      if (valHas(vAlign, "height")) {
        float heightValue = getFloat(vAlign["height"], 0.0F);
        bool heightIsPct = getBool(vAlign["heightIsPct"], false);
        pos.height = common_renderer::PositionValue{heightValue, heightIsPct};
      }
    } else if (vAlignType == "edge") {
      if (valHas(vAlign, "top")) {
        float topValue = getFloat(vAlign["top"], 0.0F);
        bool topIsPct = getBool(vAlign["topIsPct"], false);
        pos.top = common_renderer::PositionValue{topValue, topIsPct};
      }
      if (valHas(vAlign, "bottom")) {
        float bottomValue = getFloat(vAlign["bottom"], 0.0F);
        bool bottomIsPct = getBool(vAlign["bottomIsPct"], false);
        pos.bottom = common_renderer::PositionValue{bottomValue, bottomIsPct};
      }
    }
  } else {
    pos.alignV = common_renderer::VerticalAlign::TOP;
  }

  return pos;
}

// Helper function to parse stroke from skjson::Value
common_renderer::Stroke parseStroke(const skjson::Value &strokeObj) {
  common_renderer::Stroke strokeData;
  strokeData.width = getFloat(strokeObj["width"], 0.0F);
  if (valHas(strokeObj, "color")) {
    strokeData.color = parseColor(strokeObj["color"]);
  }
  if (valHas(strokeObj, "align")) {
    std::string alignStr = getString(strokeObj["align"], "center");
    strokeData.align = parseStrokeAlign(alignStr);
  } else {
    // Default to CENTER
    strokeData.align = common_renderer::StrokeAlign::CENTER;
  }
  return strokeData;
}

// Helper function to parse trim from skjson::Value
common_renderer::Trim parseTrim(const skjson::Value &trimObj) {
  common_renderer::Trim trimData;
  trimData.start = getFloat(trimObj["start"], 0.0F);
  trimData.end = getFloat(trimObj["end"], 1.0F);
  return trimData;
}

// Helper function to parse children array from skjson::Value
std::vector<std::shared_ptr<common_renderer::AnyNode>>
parseChildren(const skjson::Value &childrenVal, float parentWidth,
              float parentHeight) {
  std::vector<std::shared_ptr<common_renderer::AnyNode>> children;

  if (childrenVal.getType() == skjson::Value::Type::kNull) {
    return children;
  }

  if (childrenVal.getType() == skjson::Value::Type::kArray) {
    const auto &arr = childrenVal.as<skjson::ArrayValue>();

    // Calculate current node's dimensions for children
    // We need to get the parent node to calculate dimensions, but we don't have
    // it here So we'll use parentWidth/parentHeight directly for now Actually,
    // we need the current node to calculate its dimensions For now, let's
    // assume parentWidth/parentHeight are the current node's dimensions

    for (auto childVal : arr) {
      if (childVal.getType() != skjson::Value::Type::kNull) {
        auto childNode = parseNode(childVal, parentWidth, parentHeight);
        if (childNode) {
          children.push_back(childNode);
        }
      }
    }
  }
  return children;
}

// Helper function to parse path from skjson::Value
SkPath parsePath(const skjson::Value &pathObj) {
  SkPath skPath;
  if (valHas(pathObj, "commands")) {
    const skjson::Value &commandsVal = pathObj["commands"];
    if (commandsVal.getType() == skjson::Value::Type::kArray) {
      const auto &commands = commandsVal.as<skjson::ArrayValue>();
      for (auto cmdVal : commands) {
        if (cmdVal.getType() == skjson::Value::Type::kNull) {
          continue;
        }

        std::string typeStr = getString(cmdVal["type"], "");
        if (typeStr == "move") {
          float x = getFloat(cmdVal["x"], 0.0F);
          float y = getFloat(cmdVal["y"], 0.0F);
          skPath.moveTo(x, y);
        } else if (typeStr == "line") {
          float x = getFloat(cmdVal["x"], 0.0F);
          float y = getFloat(cmdVal["y"], 0.0F);
          skPath.lineTo(x, y);
        } else if (typeStr == "cubic") {
          float x1 = getFloat(cmdVal["x1"], 0.0F);
          float y1 = getFloat(cmdVal["y1"], 0.0F);
          float x2 = getFloat(cmdVal["x2"], 0.0F);
          float y2 = getFloat(cmdVal["y2"], 0.0F);
          float x = getFloat(cmdVal["x"], 0.0F);
          float y = getFloat(cmdVal["y"], 0.0F);
          skPath.cubicTo(x1, y1, x2, y2, x, y);
        } else if (typeStr == "close") {
          skPath.close();
        }
      }
    }
  }
  return skPath;
}

// Helper function to parse effect from skjson::Value
common_renderer::Effect parseEffect(const skjson::Value &effectObj) {
  std::string effectType = getString(effectObj["type"], "");

  if (effectType == "blur") {
    common_renderer::Blur blur;
    blur.blur = getFloat(effectObj["blur"], 0.0F);
    return {blur};
  } else if (effectType == "displacementMap") {
    common_renderer::DisplacementMap displacementMap;
    displacementMap.amplitude = getFloat(effectObj["amplitude"], 0.0F);
    displacementMap.freqX = getFloat(effectObj["freqX"], 0.0F);
    displacementMap.freqY = getFloat(effectObj["freqY"], 0.0F);
    displacementMap.octaves = getInt(effectObj["octaves"], 1);
    return {displacementMap};
  } else if (effectType == "crt") {
    common_renderer::CRTEffect crtEffect;
    crtEffect.verticalBleed = getFloat(effectObj["verticalBleed"], 0.5F);
    crtEffect.horizontalBleed = getFloat(effectObj["horizontalBleed"], 0.5F);
    crtEffect.contrast = getFloat(effectObj["contrast"], 0.5F);
    crtEffect.granularity = getFloat(effectObj["granularity"], 2.0F);
    return {crtEffect};
  } else if (effectType == "liquidGlass") {
    common_renderer::LiquidGlass liquidGlass;
    liquidGlass.refraction = getFloat(effectObj["refraction"], 1.5F);
    liquidGlass.tint = getFloat(effectObj["tint"], 0.0F);
    liquidGlass.rim = getFloat(effectObj["rim"], 10.0F);
    liquidGlass.cornerRadius = getFloat(effectObj["cornerRadius"], 0.0F);
    return {liquidGlass};
  } else {
    // Default to blur if type is unknown
    common_renderer::Blur blur;
    blur.blur = 0.0F;
    return {blur};
  }
}

// Helper function to parse effects array from skjson::Value
std::vector<common_renderer::Effect>
parseEffects(const skjson::Value &effectsVal) {
  std::vector<common_renderer::Effect> effects;
  if (effectsVal.getType() == skjson::Value::Type::kNull) {
    return effects;
  }

  if (effectsVal.getType() == skjson::Value::Type::kArray) {
    const auto &arr = effectsVal.as<skjson::ArrayValue>();
    for (auto effectVal : arr) {
      if (effectVal.getType() != skjson::Value::Type::kNull) {
        effects.push_back(parseEffect(effectVal));
      }
    }
  }
  return effects;
}

// Helper function to parse clip from skjson::Value
common_renderer::Clip parseClip(const skjson::Value &clipObj) {
  bool enable = getBool(clipObj["enable"], false);
  std::string typeStr = getString(clipObj["type"], "rrect");

  if (typeStr == "rrect") {
    common_renderer::RRectClip rrectClip;
    rrectClip.enable = enable;
    if (valHas(clipObj, "r")) {
      const skjson::Value &rVal = clipObj["r"];
      if (rVal.getType() == skjson::Value::Type::kArray) {
        // Array of radii
        const auto &rArr = rVal.as<skjson::ArrayValue>();
        size_t length = rArr.size();
        for (size_t i = 0; i < 4 && i < length; ++i) {
          rrectClip.r.at(i) = getFloat(rArr[i], 0.0F);
        }
      } else if (rVal.getType() == skjson::Value::Type::kNumber) {
        // Single number - apply to all corners
        float rValue = getFloat(rVal, 0.0F);
        rrectClip.r = {rValue, rValue, rValue, rValue};
      }
    }
    return {rrectClip};
  } else if (typeStr == "oval") {
    common_renderer::OvalClip ovalClip;
    ovalClip.enable = enable;
    return {ovalClip};
  } else if (typeStr == "path") {
    common_renderer::PathClip pathClip;
    pathClip.enable = enable;
    if (valHas(clipObj, "path")) {
      pathClip.path = parsePath(clipObj["path"]);
    }
    return {pathClip};
  } else {
    // Default to rrect if type is unknown
    common_renderer::RRectClip rrectClip;
    rrectClip.enable = enable;
    rrectClip.r = {0.0F, 0.0F, 0.0F, 0.0F};
    return {rrectClip};
  }
}

// Helper function to parse text section from skjson::Value
common_renderer::TextSection
parseTextSection(const skjson::Value &textSectionObj) {
  common_renderer::TextSection textSection;
  textSection.txt = getString(textSectionObj["txt"], "");
  textSection.fontFamily = getString(textSectionObj["fontFamily"], "");
  textSection.fontSize = getFloat(textSectionObj["fontSize"], 16.0F);

  // Parse font style
  if (valHas(textSectionObj, "fontStyle")) {
    const skjson::Value &fontStyle = textSectionObj["fontStyle"];
    std::string slantStr = getString(fontStyle["slant"], "upright");
    SkFontStyle::Slant slant = (slantStr == "italic")
                                   ? SkFontStyle::Slant::kItalic_Slant
                                   : SkFontStyle::Slant::kUpright_Slant;
    textSection.fontStyle =
        common_renderer::FontStyle{getInt(fontStyle["weight"], 400), slant};
  } else {
    textSection.fontStyle =
        common_renderer::FontStyle{400, SkFontStyle::Slant::kUpright_Slant};
  }

  // Parse foreground color
  if (valHas(textSectionObj, "foregroundColor")) {
    textSection.foregroundColor = parseColor(textSectionObj["foregroundColor"]);
  } else {
    common_renderer::SolidColor solidColor;
    solidColor.color = "#000000";
    textSection.foregroundColor = common_renderer::Color(solidColor);
  }

  textSection.letterSpacing = getFloat(textSectionObj["letterSpacing"], 0.0F);
  textSection.wordSpacing = getFloat(textSectionObj["wordSpacing"], 0.0F);
  textSection.lineHeight = getFloat(textSectionObj["lineHeight"], 1.0F);

  // Parse text decoration
  if (valHas(textSectionObj, "decoration")) {
    const skjson::Value &decoration = textSectionObj["decoration"];
    common_renderer::TextDecoration textDecoration;

    std::string typeStr = getString(decoration["type"], "none");
    textDecoration.type = parseTextDecorationType(typeStr);
    textDecoration.color = getString(decoration["color"], "#000000");
    textDecoration.thickness = getFloat(decoration["thickness"], 1.0F);

    std::string styleStr = getString(decoration["style"], "solid");
    textDecoration.style = parseTextDecorationStyle(styleStr);

    textSection.decoration = textDecoration;
  }

  return textSection;
}

// Main function to parse AnyNode from JSON string
std::shared_ptr<common_renderer::AnyNode>
parseNode(const std::string &jsonString, float parentWidth,
          float parentHeight) {
  if (jsonString.empty()) {
    return nullptr;
  }

  skjson::DOM dom(jsonString.c_str(), jsonString.size());
  const skjson::Value &root = dom.root();

  if (root.getType() == skjson::Value::Type::kNull) {
    return nullptr;
  }

  return parseNode(root, parentWidth, parentHeight);
}

// Main function to parse AnyNode from skjson::Value (internal use)
std::shared_ptr<common_renderer::AnyNode>
parseNode(const skjson::Value &json, float parentWidth, float parentHeight) {
  std::string typeStr = getString(json["type"], "");
  std::string name = getString(json["name"], "");
  int id = getInt(json["id"], 0);

  std::shared_ptr<common_renderer::AnyNode> node;

  if (typeStr == "Paragraph") {
    auto paragraphNode = std::make_shared<common_renderer::ParagraphNode>();
    paragraphNode->name = name;
    paragraphNode->id = id;

    // Parse rotation
    if (valHas(json, "rotation")) {
      paragraphNode->rotation = parseRotation(json["rotation"]);
    }

    // Parse positioning
    applyPositioning(paragraphNode.get(), parsePositioning(json));

    // Parse textAlign
    std::string alignStr = getString(json["textAlign"], "left");
    if (alignStr == "right") {
      paragraphNode->textAlign = skia::textlayout::TextAlign::kRight;
    } else if (alignStr == "center") {
      paragraphNode->textAlign = skia::textlayout::TextAlign::kCenter;
    } else {
      paragraphNode->textAlign = skia::textlayout::TextAlign::kLeft;
    }

    // Parse textOverflow
    std::string wrapStr = getString(json["textOverflow"], "clip");
    paragraphNode->textOverflow = (wrapStr == "Wrap");

    // Parse cursorOffset
    if (valHas(json, "cursorOffset")) {
      paragraphNode->cursorOffset = getInt(json["cursorOffset"], 0);
    }

    // Parse texts array
    if (valHas(json, "texts")) {
      const skjson::Value &textsVal = json["texts"];
      if (textsVal.getType() == skjson::Value::Type::kArray) {
        const auto &texts = textsVal.as<skjson::ArrayValue>();
        for (auto textVal : texts) {
          if (textVal.getType() != skjson::Value::Type::kNull) {
            paragraphNode->texts.push_back(parseTextSection(textVal));
          }
        }
      }
    }

    node = paragraphNode;
  } else if (typeStr == "Layer") {
    auto layerNode = std::make_shared<common_renderer::LayerNode>();
    layerNode->name = name;
    layerNode->id = id;

    // Parse rotation
    if (valHas(json, "rotation")) {
      layerNode->rotation = parseRotation(json["rotation"]);
    }

    // Parse positioning
    applyPositioning(layerNode.get(), parsePositioning(json));

    // Parse stroke
    if (valHas(json, "stroke")) {
      layerNode->stroke = parseStroke(json["stroke"]);
    }

    // Parse fill
    if (valHas(json, "fill")) {
      layerNode->fill = parseFill(json["fill"]);
    }

    // Parse clip
    if (valHas(json, "clip")) {
      layerNode->clip = parseClip(json["clip"]);
    }

    // Parse trim
    if (valHas(json, "trim")) {
      layerNode->trim = parseTrim(json["trim"]);
    }

    // Parse foregroundEffects
    if (valHas(json, "foregroundEffects")) {
      layerNode->foregroundEffects = parseEffects(json["foregroundEffects"]);
    }

    // Parse backdropEffects
    if (valHas(json, "backdropEffects")) {
      layerNode->backdropEffects = parseEffects(json["backdropEffects"]);
    }

    // Parse opacity
    if (valHas(json, "opacity")) {
      layerNode->opacity = getFloat(json["opacity"], 1.0F);
    }

    node = layerNode;
  } else if (typeStr == "Screen") {
    auto screenNode = std::make_shared<common_renderer::ScreenNode>();
    screenNode->name = name;
    screenNode->id = id;

    // Parse positioning
    applyPositioning(screenNode.get(), parsePositioning(json));

    // Parse stroke
    if (valHas(json, "stroke")) {
      screenNode->stroke = parseStroke(json["stroke"]);
    }

    // Parse fill
    if (valHas(json, "fill")) {
      screenNode->fill = parseFill(json["fill"]);
    }

    // Parse clip
    if (valHas(json, "clip")) {
      screenNode->clip = parseClip(json["clip"]);
    }

    node = screenNode;
  } else if (typeStr == "Path") {
    auto pathNode = std::make_shared<common_renderer::PathNode>();
    pathNode->name = name;
    pathNode->id = id;

    // Parse rotation
    if (valHas(json, "rotation")) {
      pathNode->rotation = parseRotation(json["rotation"]);
    }

    // Parse positioning
    applyPositioning(pathNode.get(), parsePositioning(json));

    // Parse path
    if (valHas(json, "path")) {
      pathNode->path = parsePath(json["path"]);
    }

    // Parse stroke
    if (valHas(json, "stroke")) {
      pathNode->stroke = parseStroke(json["stroke"]);
    }

    // Parse fill
    if (valHas(json, "fill")) {
      pathNode->fill = parseFill(json["fill"]);
    }

    // Parse trim
    if (valHas(json, "trim")) {
      pathNode->trim = parseTrim(json["trim"]);
    }

    node = pathNode;
  } else if (typeStr == "SVG") {
    auto svgNode = std::make_shared<common_renderer::SVGNode>();
    svgNode->name = name;
    svgNode->id = id;

    // Parse rotation
    if (valHas(json, "rotation")) {
      svgNode->rotation = parseRotation(json["rotation"]);
    }

    // Parse positioning
    applyPositioning(svgNode.get(), parsePositioning(json));

    // Parse SVG content
    if (valHas(json, "svg")) {
      svgNode->svg = getString(json["svg"], "");
    }

    node = svgNode;
  } else if (typeStr == "Root") {
    auto rootNode = std::make_shared<common_renderer::RootNode>();
    rootNode->name = name;
    rootNode->id = id;

    // Parse pan
    if (valHas(json, "pan")) {
      const skjson::Value &panVal = json["pan"];
      if (panVal.getType() == skjson::Value::Type::kArray) {
        const auto &panArr = panVal.as<skjson::ArrayValue>();
        if (panArr.size() >= 2) {
          float panX = getFloat(panArr[0], 0.0F);
          float panY = getFloat(panArr[1], 0.0F);
          rootNode->pan = std::make_tuple(panX, panY);
        }
      }
    }

    // Parse scale
    if (valHas(json, "scale")) {
      const skjson::Value &scaleVal = json["scale"];
      if (scaleVal.getType() == skjson::Value::Type::kArray) {
        const auto &scaleArr = scaleVal.as<skjson::ArrayValue>();
        if (scaleArr.size() >= 2) {
          float scaleX = getFloat(scaleArr[0], 1.0F);
          float scaleY = getFloat(scaleArr[1], 1.0F);
          rootNode->scale = std::make_tuple(scaleX, scaleY);
        }
      }
    }

    // Parse origin
    if (valHas(json, "origin")) {
      const skjson::Value &originVal = json["origin"];
      if (originVal.getType() == skjson::Value::Type::kArray) {
        const auto &originArr = originVal.as<skjson::ArrayValue>();
        if (originArr.size() >= 2) {
          float originX = getFloat(originArr[0], 0.0F);
          float originY = getFloat(originArr[1], 0.0F);
          rootNode->origin = std::make_tuple(originX, originY);
        }
      }
    }

    node = rootNode;
  } else {
    // Default to base AnyNode for unsupported types
    node = std::make_shared<common_renderer::AnyNode>();
    node->type = common_renderer::NodeType::LAYER; // Default
    node->name = name;
    node->id = id;
  }

  // toTlwhPriorRotation can be called here since node is already parsed
  std::vector<float> tlwh =
      common_renderer::toTlwhPriorRotation(node, parentWidth, parentHeight);
  float w = parentWidth;
  float h = parentHeight;
  if (tlwh.size() == 4) {
    w = tlwh[2];
    h = tlwh[3];
  }

  // Parse children
  if (valHas(json, "children")) {
    const auto &childrenVal = json["children"];
    if (childrenVal.getType() == skjson::Value::Type::kArray) {
      const auto &arr = childrenVal.as<skjson::ArrayValue>();
      for (auto childVal : arr) {
        if (childVal.getType() != skjson::Value::Type::kNull) {
          auto childNode = parseNode(childVal, w, h);
          if (childNode) {
            // Only LayerNode, ScreenNode, and RootNode have children
            if (auto layerNode =
                    std::dynamic_pointer_cast<common_renderer::LayerNode>(
                        node)) {
              layerNode->children.push_back(childNode);
            } else if (auto screenNode = std::dynamic_pointer_cast<
                           common_renderer::ScreenNode>(node)) {
              screenNode->children.push_back(childNode);
            } else if (auto rootNode =
                           std::dynamic_pointer_cast<common_renderer::RootNode>(
                               node)) {
              rootNode->children.push_back(childNode);
            }
          }
        }
      }
    }
  }

  return node;
}

} // namespace skia_parser
