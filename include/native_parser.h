#pragma once

#include "common_renderer.h"
#include "modules/jsonreader/SkJSONReader.h"
#include <memory>
#include <optional>
#include <vector>

namespace skia_parser {

// Helper function to convert string to HorizontalAlign enum
common_renderer::HorizontalAlign parseHorizontalAlign(const std::string& alignStr);

// Helper function to convert string to VerticalAlign enum
common_renderer::VerticalAlign parseVerticalAlign(const std::string& alignStr);

// Helper function to convert string to StrokeAlign enum
common_renderer::StrokeAlign parseStrokeAlign(const std::string& alignStr);

// Helper function to convert string to TextDecoration enum
skia::textlayout::TextDecoration parseTextDecorationType(const std::string& typeStr);

// Helper function to convert string to TextDecorationStyle enum
skia::textlayout::TextDecorationStyle parseTextDecorationStyle(const std::string& styleStr);

// Helper function to safely get a string from skjson::Value
std::string getString(const skjson::Value& v, const std::string& defaultValue = "");

// Helper function to safely get a number from skjson::Value
float getFloat(const skjson::Value& v, float defaultValue = 0.0F);
int getInt(const skjson::Value& v, int defaultValue = 0);
bool getBool(const skjson::Value& v, bool defaultValue = false);

// Helper function to check if value has a property (for objects)
bool valHas(const skjson::Value& v, const char* key);

// Helper function to parse an image fill from skjson::Value
common_renderer::ImageFill parseImageFill(const skjson::Value& fillObj);

// Helper function to parse a solid color from skjson::Value
common_renderer::SolidColor parseSolidColor(const skjson::Value& colorObj);

// Helper function to parse a gradient color from skjson::Value
common_renderer::GradientColor parseGradientColor(const skjson::Value& colorObj);

// Helper function to parse a color from skjson::Value
std::optional<common_renderer::Color> parseColor(const skjson::Value& colorObj);

// Helper function to parse a fill from skjson::Value
common_renderer::Fill parseFill(const skjson::Value& fillObj);

// Helper function to parse rotation from skjson::Value
common_renderer::Rotation parseRotation(const skjson::Value& rotationObj);

// Helper function to parse positioning from skjson::Value
common_renderer::Positioning parsePositioning(const skjson::Value& nodeObj);

// Helper function to parse stroke from skjson::Value
common_renderer::Stroke parseStroke(const skjson::Value& strokeObj);

// Helper function to parse clip from skjson::Value
common_renderer::Clip parseClip(const skjson::Value& clipObj);

// Helper function to parse trim from skjson::Value
common_renderer::Trim parseTrim(const skjson::Value& trimObj);

// Helper function to parse path from skjson::Value
SkPath parsePath(const skjson::Value& pathObj);

// Helper function to parse effect from skjson::Value
common_renderer::Effect parseEffect(const skjson::Value& effectObj);

// Helper function to parse effects array from skjson::Value
std::vector<common_renderer::Effect> parseEffects(const skjson::Value& effectsVal);

// Helper function to parse text section from skjson::Value
common_renderer::TextSection parseTextSection(const skjson::Value& textSectionObj);

// Helper function to parse children array from skjson::Value
std::vector<std::shared_ptr<common_renderer::AnyNode>> 
parseChildren(const skjson::Value& childrenVal, float parentWidth, float parentHeight);

// Main function to parse AnyNode from JSON string
std::shared_ptr<common_renderer::AnyNode> 
parseNode(const std::string& jsonString, float parentWidth, float parentHeight);

// Main function to parse AnyNode from skjson::Value (internal use)
std::shared_ptr<common_renderer::AnyNode> 
parseNode(const skjson::Value& json, float parentWidth, float parentHeight);

} // namespace skia_parser

