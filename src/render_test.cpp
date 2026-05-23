#include "native_parser.h"
#include "render_test_base.h"
#include "render_utils.h"

namespace {
bool createTransitionImages(float width, float height,
                            sk_sp<SkImage> &sourceImage,
                            sk_sp<SkImage> &targetImage) {
  const int IMAGE_WIDTH = static_cast<int>(width);
  const int IMAGE_HEIGHT = static_cast<int>(height);

  sk_sp<SkSurface> sourceSurface =
      SkSurfaces::Raster(SkImageInfo::MakeN32Premul(IMAGE_WIDTH, IMAGE_HEIGHT));
  if (!sourceSurface) {
    return false;
  }
  sourceSurface->getCanvas()->clear(SK_ColorRED);
  sourceImage = sourceSurface->makeImageSnapshot();
  if (!sourceImage) {
    return false;
  }

  sk_sp<SkSurface> targetSurface =
      SkSurfaces::Raster(SkImageInfo::MakeN32Premul(IMAGE_WIDTH, IMAGE_HEIGHT));
  if (!targetSurface) {
    return false;
  }
  targetSurface->getCanvas()->clear(SK_ColorBLUE);
  targetImage = targetSurface->makeImageSnapshot();
  return targetImage != nullptr;
}
} // namespace

TEST_F(RenderTest, DrawCircleCenter) {
  std::string json = R"json({
    "type": "Layer",
    "name": "test_circle",
    "id": 1,
    "horizontalAlignment": { "type": "center", "width": 100 },
    "verticalAlignment": { "type": "center", "height": 200 },
    "rotation": { "cx": 0, "cy": 0, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "clip": { "type": "oval", "enable": true }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_circle_center.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test circle with stroke
TEST_F(RenderTest, DrawCircleWithTrimAndStroke) {
  std::string json = R"json({
    "type": "Layer",
    "name": "test_circle_stroke",
    "id": 2,
    "horizontalAlignment": { "type": "left", "left": 50, "width": 80 },
    "verticalAlignment": { "type": "top", "top": 50, "height": 80 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "rgba(0, 0, 0, 0)" },
    "trim": { "start": 0.2, "end": 0.8 },
    "clip": { "type": "oval", "enable": true },
    "stroke": { "width": 5, "color": { "type": "solid", "color": "#0000FF" } }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw circle with trim and stroke";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_circle_with_trim_and_stroke.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test circle positioning
TEST_F(RenderTest, DrawCircleTopLeft) {
  std::string json = R"json({
    "type": "Layer",
    "name": "test_circle",
    "id": 1,
    "horizontalAlignment": { "type": "left", "left": 10, "width": 50 },
    "verticalAlignment": { "type": "top", "top": 10, "height": 50 },
    "rotation": { "cx": 0, "cy": 0, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "clip": { "type": "oval", "enable": true }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw positioned circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_circle_top_left.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile))
        << "Failed to save positioned circle";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test circle positioning - bottom right
TEST_F(RenderTest, DrawCircleBottomRight) {
  std::string json = R"json({
    "type": "Layer",
    "name": "test_circle",
    "id": 1,
    "horizontalAlignment": { "type": "right", "right": 10, "width": 50 },
    "verticalAlignment": { "type": "bottom", "bottom": 10, "height": 50 },
    "rotation": { "cx": 0, "cy": 0, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "clip": { "type": "oval", "enable": true }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw positioned circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_circle_bottom_right.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile))
        << "Failed to save positioned circle";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test circle edge positioning
TEST_F(RenderTest, DrawCircleEdgePositioned) {
  std::string json = R"json({
    "type": "Layer",
    "name": "test_circle",
    "id": 1,
    "horizontalAlignment": { "type": "edge", "left": 20, "right": 20 },
    "verticalAlignment": { "type": "edge", "top": 20, "topIsPct": false, "bottom": 20 },
    "rotation": { "cx": 0, "cy": 0, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "clip": { "type": "oval", "enable": true }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw edge positioned circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_circle_edge_positioned.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile))
        << "Failed to save edge positioned circle";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test circle with trim (partial circle)
TEST_F(RenderTest, DrawCircleWithTrim) {
  std::string json = R"json({
    "type": "Layer",
    "name": "test_circle",
    "id": 1,
    "horizontalAlignment": { "type": "center", "width": 100 },
    "verticalAlignment": { "type": "center", "height": 200 },
    "rotation": { "cx": 0, "cy": 0, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "trim": { "start": 0, "end": 0.5 },
    "clip": { "type": "oval", "enable": true }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw trimmed circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_circle_with_trim.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile))
        << "Failed to save trimmed circle";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test circle with rotation
TEST_F(RenderTest, DrawCircleWithRotation) {
  std::string json = R"json({
    "type": "Layer",
    "name": "test_circle",
    "id": 1,
    "horizontalAlignment": { "type": "center", "width": 100 },
    "verticalAlignment": { "type": "center", "height": 200 },
    "rotation": { "cx": 1, "cy": 1, "z": -30, "skewX": 0 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "clip": { "type": "oval", "enable": true }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw rotated circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_circle_with_rotation.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile))
        << "Failed to save rotated circle";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}
TEST_F(RenderTest, DrawCircleWithGradient) {
  std::string json = R"json({
    "type": "Layer",
    "name": "gradient_circle",
    "id": 100,
    "horizontalAlignment": { "type": "center", "width": 120 },
    "verticalAlignment": { "type": "center", "height": 120 },
    "rotation": { "cx": 0, "cy": 0, "z": 0, "skewX": 0 },
    "fill": {
      "type": "gradient",
      "startColor": "#FF0000",
      "endColor": "#0000FF",
      "start": { "x": 0, "y": 0 },
      "end": { "x": 1, "y": 1 }
    },
    "clip": { "type": "oval", "enable": true }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw gradient circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_circle_with_gradient.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}
TEST_F(RenderTest, DrawCircleWithDropShadow) {
  std::string json = R"json({
    "type": "Layer",
    "name": "test_circle",
    "id": 1,
    "horizontalAlignment": { "type": "center", "width": 100 },
    "verticalAlignment": { "type": "center", "height": 200 },
    "rotation": { "cx": 0, "cy": 0, "z": 0, "skewX": 0 },
    "fill": {
      "type": "solid",
      "color": "#FF0000",
      "shadows": [
        { "type": "dropShadow", "offsetX": 5, "offsetY": 5, "blurX": 3, "blurY": 3, "color": "#000000" }
      ]
    },
    "clip": { "type": "oval", "enable": true }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw circle with drop shadow";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_circle_with_drop_shadow.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawCircleWithInnerShadow) {
  std::string json = R"json({
    "type": "Layer",
    "name": "test_circle",
    "id": 1,
    "horizontalAlignment": { "type": "center", "width": 100 },
    "verticalAlignment": { "type": "center", "height": 200 },
    "rotation": { "cx": 0, "cy": 0, "z": 0, "skewX": 0 },
    "fill": {
      "type": "solid",
      "color": "#FF0000",
      "shadows": [
        { "type": "innerShadow", "offsetX": 5, "offsetY": 5, "blurX": 3, "blurY": 3, "color": "#000000" }
      ]
    },
    "clip": { "type": "oval", "enable": true }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw circle with inner shadow";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_circle_with_inner_shadow.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawCircleStrokeWithInnerShadow) {
  std::string json = R"json({
    "type": "Layer",
    "name": "test_circle_stroke_inner_shadow",
    "id": 3,
    "horizontalAlignment": { "type": "center", "width": 100 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "oval", "enable": true },
    "fill": { "type": "solid", "color": "rgba(0, 0, 0, 0)" },
    "stroke": {
      "width": 10,
      "color": {
        "type": "solid",
        "color": "#FF0000",
        "shadows": [
          { "type": "innerShadow", "offsetX": 5, "offsetY": 5, "blurX": 3, "blurY": 3, "color": "#000000" }
        ]
      }
    }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw circle with stroke and inner shadow";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile =
      "./fixture/draw_circle_stroke_with_inner_shadow.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawNeumorphicButton) {
  std::string json = R"json({
    "type": "Layer",
    "name": "test_neumorphic_button",
    "id": 100,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 80 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "fill": {
      "type": "solid",
      "color": "#E0E0E0",
      "shadows": [
        { "type": "dropShadow", "offsetX": -8, "offsetY": -8, "blurX": 12, "blurY": 12, "color": "#FFFFFF" },
        { "type": "dropShadow", "offsetX": 8, "offsetY": 8, "blurX": 12, "blurY": 12, "color": "#A0A0A0" }
      ]
    },
    "clip": { "type": "rrect", "enable": true, "r": 15 }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw neumorphic button with dual shadows";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_neumorphic_button.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawNeumorphicButtonRecessed) {
  std::string json = R"json({
    "type": "Layer",
    "name": "test_recessed_neumorphic_button",
    "id": 101,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 80 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "fill": {
      "type": "solid",
      "color": "#E0E0E0",
      "shadows": [
        { "type": "innerShadow", "offsetX": -6, "offsetY": -6, "blurX": 10, "blurY": 10, "color": "#FFFFFF" },
        { "type": "innerShadow", "offsetX": 6, "offsetY": 6, "blurX": 10, "blurY": 10, "color": "#A0A0A0" }
      ]
    },
    "clip": { "type": "rrect", "enable": true, "r": 15 }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw recessed neumorphic button with dual inner shadows";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_neumorphic_button_recessed.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test circle with both fill and stroke
TEST_F(RenderTest, DrawCircleFillAndStroke) {
  std::string json = R"json({
    "type": "Layer",
    "name": "test_circle_fill_stroke",
    "id": 1,
    "horizontalAlignment": { "type": "center", "width": 80 },
    "verticalAlignment": { "type": "center", "height": 80 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "oval", "enable": true },
    "fill": { "type": "solid", "color": "#FF0000" },
    "stroke": { "width": 4, "color": { "type": "solid", "color": "#0000FF" } }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw circle with fill and stroke";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_circle_fill_and_stroke.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group rendering with circle and rectangle children - center positioned
TEST_F(RenderTest, DrawGroupWithCircleAndRectCenter) {
  std::string json = R"json({
    "type": "Layer",
    "name": "test_group",
    "id": 10,
    "horizontalAlignment": { "type": "center", "width": 300 },
    "verticalAlignment": { "type": "center", "height": 200 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "rrect", "enable": true, "r": 10 },
    "children": [
      {
        "type": "Layer",
        "name": "group_circle",
        "id": 11,
        "horizontalAlignment": { "type": "center", "width": 80 },
        "verticalAlignment": { "type": "center", "height": 80 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FF0000" },
        "clip": { "type": "oval", "enable": true }
      },
      {
        "type": "Layer",
        "name": "group_rect",
        "id": 12,
        "horizontalAlignment": { "type": "center", "width": 120 },
        "verticalAlignment": { "type": "center", "height": 60 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#0000FF" },
        "clip": { "type": "rrect", "enable": true, "r": 15 }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with circle and rectangle";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile =
      "./fixture/draw_group_with_circle_and_rect_center.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group rendering with rotation
TEST_F(RenderTest, DrawGroupWithRotation) {
  std::string json = R"json({
    "type": "Layer",
    "name": "rotated_group",
    "id": 20,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 45, "skewX": 0 },
    "clip": { "type": "rrect", "enable": false, "r": 0 },
    "children": [
      {
        "type": "Layer",
        "name": "rotated_circle",
        "id": 21,
        "horizontalAlignment": { "type": "center", "width": 60 },
        "verticalAlignment": { "type": "center", "height": 60 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#00FF00" },
        "clip": { "type": "oval", "enable": true }
      },
      {
        "type": "Layer",
        "name": "rotated_rect",
        "id": 22,
        "horizontalAlignment": { "type": "center", "width": 100 },
        "verticalAlignment": { "type": "center", "height": 40 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FFFF00" },
        "clip": { "type": "rrect", "enable": true, "r": 10 }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw rotated group";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data and save fixture
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  std::string fixtureFile = "./fixture/draw_group_with_rotation.png";
  if (!std::filesystem::exists(fixtureFile)) {
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group rendering with edge positioning
TEST_F(RenderTest, DrawGroupEdgePositioned) {
  std::string json = R"json({
    "type": "Layer",
    "name": "edge_group",
    "id": 30,
    "horizontalAlignment": { "type": "edge", "left": 20, "right": 20 },
    "verticalAlignment": { "type": "edge", "top": 30, "topIsPct": false, "bottom": 30 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "rrect", "enable": true, "r": 20 },
    "children": [
      {
        "type": "Layer",
        "name": "edge_circle",
        "id": 31,
        "horizontalAlignment": { "type": "left", "left": 20, "width": 50 },
        "verticalAlignment": { "type": "top", "top": 20, "height": 50 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#800080" },
        "clip": { "type": "oval", "enable": true }
      },
      {
        "type": "Layer",
        "name": "edge_rect",
        "id": 32,
        "horizontalAlignment": { "type": "right", "right": 20, "width": 80 },
        "verticalAlignment": { "type": "bottom", "bottom": 20, "height": 40 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FFA500" },
        "clip": { "type": "rrect", "enable": true, "r": 5 }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw edge positioned group";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data and save fixture
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  std::string fixtureFile = "./fixture/draw_group_edge_positioned.png";
  if (!std::filesystem::exists(fixtureFile)) {
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test nested groups (group within group)
TEST_F(RenderTest, DrawGroupNested) {
  std::string json = R"json({
    "type": "Layer",
    "name": "outer_group",
    "id": 40,
    "horizontalAlignment": { "type": "center", "width": 250 },
    "verticalAlignment": { "type": "center", "height": 180 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 30, "skewX": 0 },
    "clip": { "type": "rrect", "enable": true, "r": 15 },
    "children": [
      {
        "type": "Layer",
        "name": "inner_group",
        "id": 41,
        "horizontalAlignment": { "type": "center", "width": 150 },
        "verticalAlignment": { "type": "center", "height": 100 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": -15, "skewX": 0 },
        "clip": { "type": "rrect", "enable": true, "r": 8 },
        "children": [
          {
            "type": "Layer",
            "name": "nested_circle",
            "id": 42,
            "horizontalAlignment": { "type": "center", "width": 40 },
            "verticalAlignment": { "type": "center", "height": 40 },
            "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
            "fill": { "type": "solid", "color": "#00FFFF" },
            "clip": { "type": "oval", "enable": true }
          },
          {
            "type": "Layer",
            "name": "nested_rect",
            "id": 43,
            "horizontalAlignment": { "type": "center", "width": 60 },
            "verticalAlignment": { "type": "center", "height": 30 },
            "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
            "fill": { "type": "solid", "color": "#FF00FF" },
            "clip": { "type": "rrect", "enable": true, "r": 8 }
          }
        ]
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw nested groups";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data and save fixture
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  std::string fixtureFile = "./fixture/draw_group_nested.png";
  if (!std::filesystem::exists(fixtureFile)) {
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with clipping enabled and child with negative positioning
TEST_F(RenderTest, DrawGroupWithRRectClip) {
  std::string json = R"json({
    "type": "Layer",
    "name": "clipped_group",
    "id": 80,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "rrect", "enable": true, "r": 15 },
    "children": [
      {
        "type": "Layer",
        "name": "clipped_rect",
        "id": 81,
        "horizontalAlignment": { "type": "left", "left": -30, "width": 120 },
        "verticalAlignment": { "type": "top", "top": -20, "height": 80 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 20, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FF0000" },
        "clip": { "type": "rrect", "enable": true, "r": 0 }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with rrect clip";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_group_with_rrect_clip.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group without clipping enabled and child with negative positioning
TEST_F(RenderTest, DrawGroupWithoutClip) {
  std::string json = R"json({
    "type": "Layer",
    "name": "unclipped_group",
    "id": 90,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "rrect", "enable": false, "r": 15 },
    "children": [
      {
        "type": "Layer",
        "name": "unclipped_rect",
        "id": 91,
        "horizontalAlignment": { "type": "left", "left": -30, "width": 120 },
        "verticalAlignment": { "type": "top", "top": -20, "height": 80 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 20, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FF0000" },
        "clip": { "type": "rrect", "enable": true, "r": 0 }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group without clip";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_group_without_clip.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with oval clip
TEST_F(RenderTest, DrawGroupWithOvalClip) {
  std::string json = R"json({
    "type": "Layer",
    "name": "oval_clipped_group",
    "id": 95,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "oval", "enable": true },
    "children": [
      {
        "type": "Layer",
        "name": "oval_clipped_rect",
        "id": 96,
        "horizontalAlignment": { "type": "left", "left": -30, "width": 120 },
        "verticalAlignment": { "type": "top", "top": -20, "height": 80 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FF0000" },
        "clip": { "type": "rrect", "enable": true, "r": 0 }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with oval clip";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_group_with_oval_clip.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with path clip
TEST_F(RenderTest, DrawGroupWithPathClip) {
  std::string json = R"json({
    "type": "Layer",
    "name": "path_clipped_group",
    "id": 97,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": {
      "type": "path",
      "enable": true,
      "path": {
        "commands": [
          { "type": "move", "x": 0, "y": 0 },
          { "type": "line", "x": 200, "y": 0 },
          { "type": "cubic", "x1": 200, "y1": 150, "x2": 200, "y2": 150, "x": 0, "y": 150 },
          { "type": "close" }
        ]
      }
    },
    "children": [
      {
        "type": "Layer",
        "name": "path_clipped_rect",
        "id": 98,
        "horizontalAlignment": { "type": "edge", "left": 0, "right": 0 },
        "verticalAlignment": { "type": "edge", "top": 0, "topIsPct": false, "bottom": 0 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FF0000" },
        "clip": { "type": "rrect", "enable": true, "r": 0 }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with path clip";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_group_with_path_clip.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with path clip and trim
TEST_F(RenderTest, DrawGroupWithPathClipAndTrim) {
  std::string json = R"json({
    "type": "Layer",
    "name": "path_clipped_trimmed_group",
    "id": 108,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": {
      "type": "path",
      "enable": true,
      "path": {
        "commands": [
          { "type": "move", "x": 0, "y": 0 },
          { "type": "line", "x": 200, "y": 0 },
          { "type": "cubic", "x1": 200, "y1": 150, "x2": 200, "y2": 150, "x": 0, "y": 150 },
          { "type": "close" }
        ]
      }
    },
    "trim": { "start": 0.2, "end": 0.8 },
    "stroke": { "width": 4, "color": { "type": "solid", "color": "#0000FF" } },
    "children": [
      {
        "type": "Layer",
        "name": "path_clipped_trimmed_rect",
        "id": 109,
        "horizontalAlignment": { "type": "edge", "left": 0, "right": 0 },
        "verticalAlignment": { "type": "edge", "top": 0, "topIsPct": false, "bottom": 0 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FF0000" },
        "clip": { "type": "rrect", "enable": true, "r": 0 }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with path clip and trim";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_group_with_path_clip_and_trim.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with trim and no clip
TEST_F(RenderTest, DrawGroupWithTrimNoClip) {
  std::string json = R"json({
    "type": "Layer",
    "name": "trimmed_no_clip_group",
    "id": 110,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "rrect", "enable": false, "r": 0 },
    "trim": { "start": 0.2, "end": 0.8 },
    "stroke": { "width": 4, "color": { "type": "solid", "color": "#0000FF" } },
    "children": [
      {
        "type": "Layer",
        "name": "trimmed_no_clip_rect",
        "id": 111,
        "horizontalAlignment": { "type": "edge", "left": 0, "right": 0 },
        "verticalAlignment": { "type": "edge", "top": 0, "topIsPct": false, "bottom": 0 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FF0000" },
        "clip": { "type": "rrect", "enable": true, "r": 0 }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with trim no clip";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_group_with_trim_no_clip.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with rrect clip and stroke
TEST_F(RenderTest, DrawGroupWithRRectClipAndStroke) {
  std::string json = R"json({
    "type": "Layer",
    "name": "rrect_clipped_stroked_group",
    "id": 100,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "rrect", "enable": true, "r": 15 },
    "stroke": { "width": 4, "color": { "type": "solid", "color": "#0000FF" } },
    "children": [
      {
        "type": "Layer",
        "name": "rrect_clipped_stroked_rect",
        "id": 101,
        "horizontalAlignment": { "type": "left", "left": -30, "width": 120 },
        "verticalAlignment": { "type": "top", "top": -20, "height": 80 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 20, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FF0000" },
        "clip": { "type": "rrect", "enable": true, "r": 0 }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with rrect clip and stroke";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile =
      "./fixture/draw_group_with_rrect_clip_and_stroke.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with oval clip and stroke
TEST_F(RenderTest, DrawGroupWithOvalClipAndStroke) {
  std::string json = R"json({
    "type": "Layer",
    "name": "oval_clipped_stroked_group",
    "id": 102,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "oval", "enable": true },
    "stroke": { "width": 4, "color": { "type": "solid", "color": "#0000FF" } },
    "children": [
      {
        "type": "Layer",
        "name": "oval_clipped_stroked_rect",
        "id": 103,
        "horizontalAlignment": { "type": "left", "left": -30, "width": 120 },
        "verticalAlignment": { "type": "top", "top": -20, "height": 80 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FF0000" },
        "clip": { "type": "rrect", "enable": true, "r": 0 }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with oval clip and stroke";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile =
      "./fixture/draw_group_with_oval_clip_and_stroke.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with path clip and stroke
TEST_F(RenderTest, DrawGroupWithPathClipAndStroke) {
  std::string json = R"json({
    "type": "Layer",
    "name": "path_clipped_stroked_group",
    "id": 104,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": {
      "type": "path",
      "enable": true,
      "path": {
        "commands": [
          { "type": "move", "x": 0, "y": 0 },
          { "type": "line", "x": 200, "y": 0 },
          { "type": "cubic", "x1": 200, "y1": 150, "x2": 200, "y2": 150, "x": 0, "y": 150 },
          { "type": "close" }
        ]
      }
    },
    "stroke": { "width": 4, "color": { "type": "solid", "color": "#0000FF" } },
    "children": [
      {
        "type": "Layer",
        "name": "path_clipped_stroked_rect",
        "id": 105,
        "horizontalAlignment": { "type": "edge", "left": 0, "right": 0 },
        "verticalAlignment": { "type": "edge", "top": 0, "topIsPct": false, "bottom": 0 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FF0000" },
        "clip": { "type": "rrect", "enable": true, "r": 0 }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";

  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with path clip and stroke";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile =
      "./fixture/draw_group_with_path_clip_and_stroke.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with fill, stroke, and clip
TEST_F(RenderTest, DrawGroupWithFillStrokeAndClip) {
  std::string json = R"json({
    "type": "Layer",
    "name": "fill_stroke_clipped_group",
    "id": 106,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "rrect", "enable": true, "r": 15 },
    "fill": { "type": "solid", "color": "#FFFF00" },
    "stroke": { "width": 4, "color": { "type": "solid", "color": "#0000FF" } },
    "children": [
      {
        "type": "Layer",
        "name": "fill_stroke_clipped_rect",
        "id": 107,
        "horizontalAlignment": { "type": "left", "left": -30, "width": 120 },
        "verticalAlignment": { "type": "top", "top": -20, "height": 80 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 20, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FF0000" },
        "clip": { "type": "rrect", "enable": true, "r": 0 }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with fill stroke and clip";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile =
      "./fixture/draw_group_with_fill_stroke_and_clip.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with fill color
TEST_F(RenderTest, DrawGroupWithFill) {
  std::string json = R"json({
    "type": "Layer",
    "name": "group_with_fill",
    "id": 200,
    "horizontalAlignment": { "type": "center", "width": 250 },
    "verticalAlignment": { "type": "center", "height": 180 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 20, "skewX": 0 },
    "clip": { "type": "rrect", "enable": false, "r": 0 },
    "fill": {
      "type": "solid",
      "color": "#FFFF00",
      "shadows": [{ "type": "dropShadow", "offsetX": 4, "offsetY": 4, "blurX": 6, "blurY": 6, "color": "#000000" }]
    },
    "children": [
      {
        "type": "Layer",
        "name": "fill_test_circle",
        "id": 201,
        "horizontalAlignment": { "type": "center", "width": 100 },
        "verticalAlignment": { "type": "center", "height": 100 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FF0000" },
        "clip": { "type": "oval", "enable": true }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with fill";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_group_with_fill.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with gradient stroke color
TEST_F(RenderTest, DrawGroupWithStroke) {
  std::string json = R"json({
    "type": "Layer",
    "name": "group_with_stroke",
    "id": 210,
    "horizontalAlignment": { "type": "center", "width": 220 },
    "verticalAlignment": { "type": "center", "height": 160 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 20, "skewX": 0 },
    "clip": { "type": "rrect", "enable": false, "r": 0 },
    "stroke": {
      "width": 6,
      "color": {
        "type": "gradient",
        "startColor": "#00FF00",
        "endColor": "#FF00FF",
        "start": { "x": 0, "y": 0 },
        "end": { "x": 1, "y": 1 },
        "shadows": [{ "type": "dropShadow", "offsetX": 3, "offsetY": 3, "blurX": 5, "blurY": 5, "color": "#000000" }]
      }
    },
    "children": [
      {
        "type": "Layer",
        "name": "stroke_test_circle",
        "id": 211,
        "horizontalAlignment": { "type": "center", "width": 80 },
        "verticalAlignment": { "type": "center", "height": 80 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FFFFFF" },
        "clip": { "type": "oval", "enable": true }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with stroke";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_group_with_stroke.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with outside stroke alignment
TEST_F(RenderTest, DrawGroupWithOutsideStroke) {
  std::string json = R"json({
    "type": "Layer",
    "name": "outside_stroke_group",
    "id": 300,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "rrect", "enable": true, "r": 20 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "stroke": {
      "width": 8,
      "align": "outside",
      "color": { "type": "solid", "color": "#0000FF" }
    }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with outside stroke";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_group_with_outside_stroke.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with inside stroke alignment
TEST_F(RenderTest, DrawGroupWithInsideStroke) {
  std::string json = R"json({
    "type": "Layer",
    "name": "inside_stroke_group",
    "id": 302,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "rrect", "enable": true, "r": 20 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "stroke": {
      "width": 8,
      "align": "inside",
      "color": { "type": "solid", "color": "#0000FF" }
    },
    "children": [
      {
        "type": "Layer",
        "name": "inside_stroke_rect",
        "id": 303,
        "horizontalAlignment": { "type": "edge", "left": 0, "right": 0 },
        "verticalAlignment": { "type": "edge", "top": 0, "bottom": 0 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FF0000" },
        "clip": { "type": "rrect", "enable": true, "r": 0 }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with inside stroke";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_group_with_inside_stroke.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with oval clip and inside stroke alignment
TEST_F(RenderTest, DrawGroupOvalWithInsideStroke) {
  std::string json = R"json({
    "type": "Layer",
    "name": "oval_inside_stroke_group",
    "id": 301,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "oval", "enable": true },
    "fill": { "type": "solid", "color": "#FF0000" },
    "stroke": {
      "width": 8,
      "align": "inside",
      "color": { "type": "solid", "color": "#0000FF" }
    },
    "children": [
      {
        "type": "Layer",
        "name": "oval_inside_stroke_rect",
        "id": 304,
        "horizontalAlignment": { "type": "edge", "left": 0, "right": 0 },
        "verticalAlignment": { "type": "edge", "top": 0, "bottom": 0 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FF0000" },
        "clip": { "type": "rrect", "enable": true, "r": 0 }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result)
      << "Failed to draw group with oval clip and inside stroke";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_group_oval_with_inside_stroke.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with oval clip and outside stroke alignment
TEST_F(RenderTest, DrawGroupOvalWithOutsideStroke) {
  std::string json = R"json({
    "type": "Layer",
    "name": "oval_outside_stroke_group",
    "id": 302,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "oval", "enable": true },
    "fill": { "type": "solid", "color": "#FF0000" },
    "stroke": {
      "width": 8,
      "align": "outside",
      "color": { "type": "solid", "color": "#0000FF" }
    }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result)
      << "Failed to draw group with oval clip and outside stroke";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_group_oval_with_outside_stroke.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with backdrop blur
TEST_F(RenderTest, DrawGroupWithBackdropBlur) {
  std::string blackRectJson = R"json({
    "type": "Layer",
    "name": "black_background_rect",
    "id": 401,
    "horizontalAlignment": { "type": "center", "width": 300 },
    "verticalAlignment": { "type": "center", "height": 40 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#000000" },
    "clip": { "type": "rrect", "enable": true, "r": 10 }
  })json";
  auto blackRect = skia_parser::parseNode(blackRectJson, testWidth, testHeight);
  ASSERT_NE(blackRect, nullptr) << "Failed to parse black rect";
  bool r1 = common_renderer::draw(testCanvas, blackRect, testWidth, testHeight, false);
  EXPECT_TRUE(r1) << "Failed to draw black background rect";

  std::string groupJson = R"json({
    "type": "Layer",
    "name": "backdrop_blur_group",
    "id": 400,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "rrect", "enable": true, "r": 0 },
    "opacity": 0.5,
    "backdropEffects": [{ "type": "blur", "blur": 10 }],
    "children": [
      {
        "type": "Layer",
        "name": "green_circle",
        "id": 402,
        "horizontalAlignment": { "type": "left", "left": -10, "width": 80 },
        "verticalAlignment": { "type": "top", "top": -10, "height": 80 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#00FF00" },
        "clip": { "type": "oval", "enable": true }
      }
    ]
  })json";
  auto group = skia_parser::parseNode(groupJson, testWidth, testHeight);
  ASSERT_NE(group, nullptr) << "Failed to parse group";
  bool result =
      common_renderer::draw(testCanvas, group, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with backdrop blur";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_group_with_backdrop_blur.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with foreground blur
TEST_F(RenderTest, DrawGroupWithForegroundBlur) {
  std::string blackRectJson = R"json({
    "type": "Layer",
    "name": "black_background_rect",
    "id": 601,
    "horizontalAlignment": { "type": "center", "width": 300 },
    "verticalAlignment": { "type": "center", "height": 40 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#000000" },
    "clip": { "type": "rrect", "enable": true, "r": 10 }
  })json";
  auto blackRect = skia_parser::parseNode(blackRectJson, testWidth, testHeight);
  ASSERT_NE(blackRect, nullptr) << "Failed to parse black rect";
  bool r1 = common_renderer::draw(testCanvas, blackRect, testWidth, testHeight, false);
  EXPECT_TRUE(r1) << "Failed to draw black background rect";

  std::string groupJson = R"json({
    "type": "Layer",
    "name": "foreground_blur_group",
    "id": 600,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 200 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "rrect", "enable": false, "r": 0 },
    "foregroundEffects": [{ "type": "blur", "blur": 10 }],
    "children": [
      {
        "type": "Layer",
        "name": "green_circle",
        "id": 602,
        "horizontalAlignment": { "type": "center", "width": 80 },
        "verticalAlignment": { "type": "center", "height": 80 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#00FF00" },
        "clip": { "type": "oval", "enable": true }
      }
    ]
  })json";
  auto group = skia_parser::parseNode(groupJson, testWidth, testHeight);
  ASSERT_NE(group, nullptr) << "Failed to parse group";
  bool result =
      common_renderer::draw(testCanvas, group, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with foreground blur";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_group_with_foreground_blur.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test group with opacity
TEST_F(RenderTest, DrawGroupWithOpacity) {
  std::string blackRectJson = R"json({
    "type": "Layer",
    "name": "black_background_rect",
    "id": 701,
    "horizontalAlignment": { "type": "center", "width": 300 },
    "verticalAlignment": { "type": "center", "height": 40 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#000000" },
    "clip": { "type": "rrect", "enable": true, "r": 10 }
  })json";
  auto blackRect = skia_parser::parseNode(blackRectJson, testWidth, testHeight);
  ASSERT_NE(blackRect, nullptr) << "Failed to parse black rect";
  bool r1 = common_renderer::draw(testCanvas, blackRect, testWidth, testHeight, false);
  EXPECT_TRUE(r1) << "Failed to draw black background rect";

  std::string groupJson = R"json({
    "type": "Layer",
    "name": "opacity_group",
    "id": 700,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 200 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "clip": { "type": "rrect", "enable": false, "r": 0 },
    "opacity": 0.5,
    "children": [
      {
        "type": "Layer",
        "name": "green_circle",
        "id": 702,
        "horizontalAlignment": { "type": "center", "width": 80 },
        "verticalAlignment": { "type": "center", "height": 80 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#00FF00" },
        "clip": { "type": "oval", "enable": true }
      }
    ]
  })json";
  auto group = skia_parser::parseNode(groupJson, testWidth, testHeight);
  ASSERT_NE(group, nullptr) << "Failed to parse group";
  bool result =
      common_renderer::draw(testCanvas, group, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw group with opacity";
  EXPECT_TRUE(drawStateTestCircle()) << "Failed to draw state test circle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_group_with_opacity.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImage) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group","id":10,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover"}})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageWithClip) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_with_clip","id":11,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover"},"clip":{"type":"rrect","enable":true,"r":30}})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group with clip";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_with_clip.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageWithStroke) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_with_stroke","id":12,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover"},"stroke":{"width":4,"color":{"type":"solid","color":"#0000FF"}}})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group with stroke";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_with_stroke.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageContain) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_contain","id":13,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"contain"}})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group with contain mode";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_contain.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageCover) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_cover","id":14,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover"}})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group with cover mode";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_cover.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageRepeat) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_repeat","id":15,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"repeat","scaleBeforeRepeat":0.01}})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group with repeat mode";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_repeat.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageRepeatWithRotation) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_repeat_rotation","id":23,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"repeat","scaleBeforeRepeat":0.01,"rotateBeforeRepeat":45}})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result)
      << "Failed to draw image group with repeat mode and rotation";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_repeat_with_rotation.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageWithExposure) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_exposure","id":16,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover","exposure":1.0}})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group with exposure filter";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_with_exposure.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageWithContrast) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_contrast","id":17,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover","contrast":0.9}})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group with contrast filter";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_with_contrast.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageWithSaturation) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_saturation","id":18,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover","saturation":-1.0}})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group with saturation filter";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_with_saturation.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageWithTemperature) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_temperature","id":19,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover","temperature":1.0}})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group with temperature filter";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_with_temperature.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageWithTint) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_tint","id":20,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover","tint":-1.0}})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group with tint filter";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_with_tint.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageWithHighlights) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_highlights","id":21,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover","highlights":-1.0}})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group with highlights filter";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_with_highlights.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageWithShadows) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_shadows","id":22,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover","shadows":-1.0}})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group with shadows filter";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_with_shadows.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageWithDisplacementMap) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_displacement","id":24,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover"},"clip":{"type":"rrect","enable":true,"r":0},"stroke":{"width":1,"color":{"type":"solid","color":"#000000"}},"backdropEffects":[{"type":"displacementMap","amplitude":10,"freqX":0.1,"freqY":0.01,"octaves":3}]})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group with displacement map";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_with_displacement_map.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageWithCRT) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_crt","id":25,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover"},"clip":{"type":"rrect","enable":true,"r":0},"backdropEffects":[{"type":"crt","verticalBleed":0.5,"horizontalBleed":0.5,"contrast":0.5,"granularity":2.0}]})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group with CRT effect";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_with_crt.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageWithLiquidGlass) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_liquid_glass","id":26,"horizontalAlignment":{"type":"center","width":200},"verticalAlignment":{"type":"center","height":200},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover"},"clip":{"type":"rrect","enable":true,"r":[10,30,999,15]},"backdropEffects":[{"type":"liquidGlass","refraction":1.5,"tint":0.1,"rim":20.0}]})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw image group with LiquidGlass effect";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_with_liquid_glass.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawImageWithOvalLiquidGlass) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({"type":"Layer","name":"image_group_oval_liquid_glass","id":27,"horizontalAlignment":{"type":"center","width":250},"verticalAlignment":{"type":"center","height":150},"fill":{"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover"},"clip":{"type":"oval","enable":true},"backdropEffects":[{"type":"liquidGlass","refraction":1.5,"tint":0.1,"rim":20.0}]})json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result)
      << "Failed to draw image group with Oval LiquidGlass effect";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_image_with_oval_liquid_glass.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawParagraph) {
  common_renderer::FontManagerSingleton::getInstance().setFontManager(
      MakeTestFontMgr());

  std::string json = R"json({
    "type": "Paragraph",
    "name": "test_paragraph",
    "id": 200,
    "horizontalAlignment": { "type": "center", "width": 300 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "textAlign": "center",
    "texts": [
      { "txt": "Hello ", "fontFamily": "Arial", "fontSize": 24, "fontStyle": { "weight": 400, "slant": "upright" }, "letterSpacing": 0, "wordSpacing": 0, "foregroundColor": { "type": "solid", "color": "#FF0000" } },
      { "txt": "Skia Paragraph!", "fontFamily": "Arial", "fontSize": 24, "fontStyle": { "weight": 700, "slant": "italic" }, "letterSpacing": 0, "wordSpacing": 0, "foregroundColor": { "type": "solid", "color": "#0000FF" } }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw paragraph";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_paragraph.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test screen rendering with teal background and two children
TEST_F(RenderTest, DrawParagraphWithShadow) {
  common_renderer::FontManagerSingleton::getInstance().setFontManager(
      MakeTestFontMgr());

  std::string json = R"json({
    "type": "Paragraph",
    "name": "test_paragraph_with_drop_shadow",
    "id": 201,
    "horizontalAlignment": { "type": "center", "width": 300 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "textAlign": "center",
    "texts": [
      { "txt": "Hello ", "fontFamily": "Arial", "fontSize": 24, "fontStyle": { "weight": 400, "slant": "upright" }, "letterSpacing": 0, "wordSpacing": 0, "foregroundColor": { "type": "solid", "color": "#FF0000", "shadows": [{ "type": "dropShadow", "offsetX": -6, "offsetY": -6, "blurX": 0, "blurY": 0, "color": "#000000" }] } },
      { "txt": "Skia Paragraph!", "fontFamily": "Arial", "fontSize": 24, "fontStyle": { "weight": 700, "slant": "italic" }, "letterSpacing": 0, "wordSpacing": 0, "foregroundColor": { "type": "solid", "color": "#0000FF", "shadows": [{ "type": "dropShadow", "offsetX": -6, "offsetY": -6, "blurX": 4, "blurY": 4, "color": "#000000" }] } }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw paragraph with drop shadow";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_paragraph_with_drop_shadow.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawParagraphWithInnerShadow) {
  common_renderer::FontManagerSingleton::getInstance().setFontManager(
      MakeTestFontMgr());

  std::string json = R"json({
    "type": "Paragraph",
    "name": "test_paragraph_with_inner_shadow",
    "id": 209,
    "horizontalAlignment": { "type": "center", "width": 400 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "textAlign": "center",
    "texts": [
      { "txt": "Hello\n", "fontFamily": "Arial", "fontSize": 48, "fontStyle": { "weight": 700, "slant": "upright" }, "letterSpacing": 0, "wordSpacing": 0, "foregroundColor": { "type": "solid", "color": "#FF0000", "shadows": [{ "type": "innerShadow", "offsetX": 5, "offsetY": 5, "blurX": 3, "blurY": 3, "color": "#000000" }] } },
      { "txt": "Skia Paragraph!", "fontFamily": "Arial", "fontSize": 48, "fontStyle": { "weight": 900, "slant": "italic" }, "letterSpacing": 0, "wordSpacing": 0, "foregroundColor": { "type": "solid", "color": "#0000FF", "shadows": [{ "type": "innerShadow", "offsetX": 5, "offsetY": 5, "blurX": 4, "blurY": 4, "color": "#000000" }] } }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw paragraph with inner shadow";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_paragraph_with_inner_shadow.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test paragraph with text decoration and spacing
TEST_F(RenderTest, DrawParagraphWithDecoration) {
  common_renderer::FontManagerSingleton::getInstance().setFontManager(
      MakeTestFontMgr());

  std::string json = R"json({
    "type": "Paragraph",
    "name": "test_paragraph_with_decoration",
    "id": 202,
    "horizontalAlignment": { "type": "center", "width": 400 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "textAlign": "center",
    "texts": [
      { "txt": "Underlined ", "fontFamily": "Arial", "fontSize": 24, "fontStyle": { "weight": 400, "slant": "upright" }, "letterSpacing": 1, "wordSpacing": 0, "foregroundColor": { "type": "solid", "color": "#000000" }, "decoration": { "type": "underline", "color": "#FF0000", "thickness": 2, "style": "solid" } },
      { "txt": "Strikethrough ", "fontFamily": "Arial", "fontSize": 24, "fontStyle": { "weight": 700, "slant": "upright" }, "letterSpacing": 0, "wordSpacing": 2, "foregroundColor": { "type": "solid", "color": "#333333" }, "decoration": { "type": "line-through", "color": "#0000FF", "thickness": 1.5, "style": "dashed" } },
      { "txt": "Overlined", "fontFamily": "Arial", "fontSize": 24, "fontStyle": { "weight": 400, "slant": "italic" }, "letterSpacing": 0.5, "wordSpacing": 1, "foregroundColor": { "type": "solid", "color": "#666666" }, "decoration": { "type": "overline", "color": "#00FF00", "thickness": 1, "style": "dotted" } }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw paragraph with decoration";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_paragraph_with_decoration.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test paragraph with multiple sections
TEST_F(RenderTest, DrawParagraphWithMultipleSections) {
  common_renderer::FontManagerSingleton::getInstance().setFontManager(
      MakeTestFontMgr());

  std::string json = R"json({
    "type": "Paragraph",
    "name": "test_paragraph_with_multiple_sections",
    "id": 203,
    "horizontalAlignment": { "type": "center", "width": 400 },
    "verticalAlignment": { "type": "center", "height": 300 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "textAlign": "left",
    "texts": [
      { "txt": "Line 1 with normal line height\n", "fontFamily": "Arial", "fontSize": 16, "fontStyle": { "weight": 100, "slant": "upright" }, "letterSpacing": 2, "wordSpacing": 2, "lineHeight": 1.0, "foregroundColor": { "type": "solid", "color": "#000000" } },
      { "txt": "Line 2 with increased line height\n", "fontFamily": "Arial", "fontSize": 16, "fontStyle": { "weight": 400, "slant": "italic" }, "letterSpacing": 0, "wordSpacing": 0, "lineHeight": 2.0, "foregroundColor": { "type": "solid", "color": "#A00000" } },
      { "txt": "Line 3 with compact line height", "fontFamily": "Arial", "fontSize": 16, "fontStyle": { "weight": 900, "slant": "upright" }, "letterSpacing": 1, "wordSpacing": 1, "lineHeight": 0.5, "foregroundColor": { "type": "solid", "color": "#0000A0" } }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw paragraph with line height";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile =
      "./fixture/draw_paragraph_with_multiple_sections.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test paragraph with text wrapping enabled
TEST_F(RenderTest, DrawParagraphWithTextOverflow) {
  common_renderer::FontManagerSingleton::getInstance().setFontManager(
      MakeTestFontMgr());

  std::string json = R"json({
    "type": "Paragraph",
    "name": "test_paragraph_with_text_overflow",
    "id": 204,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 400 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "textAlign": "left",
    "textOverflow": "wrap",
    "texts": [
      { "txt": "This is a very long text that should wrap to multiple lines when textOverflow is enabled. The text should automatically break at word boundaries and continue on the next line when it exceeds the paragraph width. ", "fontFamily": "Arial", "fontSize": 10, "fontStyle": { "weight": 400, "slant": "upright" }, "letterSpacing": 0, "wordSpacing": 0, "lineHeight": 1.2, "foregroundColor": { "type": "solid", "color": "#000000" } },
      { "txt": "This is another very long text section that should also wrap properly. When text wrapping is enabled, the paragraph should handle long strings gracefully by breaking them across multiple lines. This ensures that text remains readable even when the container width is limited. ", "fontFamily": "Arial", "fontSize": 10, "fontStyle": { "weight": 600, "slant": "upright" }, "letterSpacing": 0, "wordSpacing": 0, "lineHeight": 1.2, "foregroundColor": { "type": "solid", "color": "#0066CC" } }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw paragraph with text wrap";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_paragraph_with_text_overflow.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test paragraph with edge alignment
TEST_F(RenderTest, DrawParagraphEdgeAligned) {
  common_renderer::FontManagerSingleton::getInstance().setFontManager(
      MakeTestFontMgr());

  std::string json1 = R"json({
    "type": "Paragraph",
    "name": "test_paragraph_edge_aligned",
    "id": 205,
    "horizontalAlignment": { "type": "edge", "left": 0, "leftIsPct": true, "right": 50, "rightIsPct": true },
    "verticalAlignment": { "type": "edge", "top": 25, "topIsPct": true, "bottom": 25, "bottomIsPct": true },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "textAlign": "center",
    "textOverflow": "wrap",
    "texts": [
      { "txt": "This paragraph is edge-aligned, filling the space between the edges. ", "fontFamily": "Arial", "fontSize": 18, "fontStyle": { "weight": 400, "slant": "upright" }, "letterSpacing": 0, "wordSpacing": 0, "lineHeight": 1.5, "foregroundColor": { "type": "solid", "color": "#000000" } },
      { "txt": "It uses horizontal edge alignment (left and right) and vertical edge alignment (top and bottom). ", "fontFamily": "Arial", "fontSize": 18, "fontStyle": { "weight": 600, "slant": "upright" }, "letterSpacing": 0, "wordSpacing": 0, "lineHeight": 1.5, "foregroundColor": { "type": "solid", "color": "#0066CC" } }
    ]
  })json";
  auto scene1 = skia_parser::parseNode(json1, testWidth, testHeight);
  ASSERT_NE(scene1, nullptr) << "Failed to parse scene1";
  bool r1 = common_renderer::draw(testCanvas, scene1, testWidth, testHeight, false);
  EXPECT_TRUE(r1) << "Failed to draw edge-aligned paragraph";

  std::string json2 = R"json({
    "type": "Paragraph",
    "name": "test_paragraph_edge_aligned_short",
    "id": 206,
    "horizontalAlignment": { "type": "edge", "left": 50, "leftIsPct": true, "right": 0, "rightIsPct": true },
    "verticalAlignment": { "type": "edge", "top": 25, "topIsPct": true, "bottom": 25, "bottomIsPct": true },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "textAlign": "center",
    "textOverflow": "wrap",
    "texts": [
      { "txt": "This paragraph is edge-aligned, filling the space between the edges. ", "fontFamily": "Arial", "fontSize": 18, "fontStyle": { "weight": 400, "slant": "upright" }, "letterSpacing": 0, "wordSpacing": 0, "lineHeight": 1.5, "foregroundColor": { "type": "solid", "color": "#000000" } }
    ]
  })json";
  auto scene2 = skia_parser::parseNode(json2, testWidth, testHeight);
  ASSERT_NE(scene2, nullptr) << "Failed to parse scene2";
  bool result =
      common_renderer::draw(testCanvas, scene2, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw edge-aligned paragraph";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_paragraph_edge_aligned.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test paragraph with bottom alignment
TEST_F(RenderTest, DrawParagraphBottomAligned) {
  common_renderer::FontManagerSingleton::getInstance().setFontManager(
      MakeTestFontMgr());

  std::string json1 = R"json({
    "type": "Paragraph",
    "name": "test_paragraph_bottom_aligned",
    "id": 207,
    "horizontalAlignment": { "type": "edge", "left": 0, "leftIsPct": true, "right": 50, "rightIsPct": true },
    "verticalAlignment": { "type": "bottom", "bottom": 12.5, "bottomIsPct": true, "height": 50, "heightIsPct": true },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "textAlign": "center",
    "textOverflow": "wrap",
    "texts": [
      { "txt": "This paragraph is bottom-aligned, positioned from the bottom edge. ", "fontFamily": "Arial", "fontSize": 18, "fontStyle": { "weight": 400, "slant": "upright" }, "letterSpacing": 0, "wordSpacing": 0, "lineHeight": 1.5, "foregroundColor": { "type": "solid", "color": "#000000" } },
      { "txt": "It uses horizontal edge alignment (left and right) and vertical bottom alignment (bottom and height). ", "fontFamily": "Arial", "fontSize": 18, "fontStyle": { "weight": 600, "slant": "upright" }, "letterSpacing": 0, "wordSpacing": 0, "lineHeight": 1.5, "foregroundColor": { "type": "solid", "color": "#0066CC" } }
    ]
  })json";
  auto scene1 = skia_parser::parseNode(json1, testWidth, testHeight);
  ASSERT_NE(scene1, nullptr) << "Failed to parse scene1";
  bool r1 = common_renderer::draw(testCanvas, scene1, testWidth, testHeight, false);
  EXPECT_TRUE(r1) << "Failed to draw bottom-aligned paragraph";

  std::string json2 = R"json({
    "type": "Paragraph",
    "name": "test_paragraph_bottom_aligned_short",
    "id": 208,
    "horizontalAlignment": { "type": "edge", "left": 50, "leftIsPct": true, "right": 0, "rightIsPct": true },
    "verticalAlignment": { "type": "bottom", "bottom": 12.5, "bottomIsPct": true, "height": 50, "heightIsPct": true },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "textAlign": "center",
    "textOverflow": "wrap",
    "texts": [
      { "txt": "This paragraph is bottom-aligned, positioned from the bottom edge. ", "fontFamily": "Arial", "fontSize": 18, "fontStyle": { "weight": 400, "slant": "upright" }, "letterSpacing": 0, "wordSpacing": 0, "lineHeight": 1.5, "foregroundColor": { "type": "solid", "color": "#000000" } }
    ]
  })json";
  auto scene2 = skia_parser::parseNode(json2, testWidth, testHeight);
  ASSERT_NE(scene2, nullptr) << "Failed to parse scene2";
  bool result =
      common_renderer::draw(testCanvas, scene2, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw bottom-aligned paragraph";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_paragraph_bottom_aligned.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test paragraph with cursor
TEST_F(RenderTest, DrawParagraphWithCursor) {
  common_renderer::FontManagerSingleton::getInstance().setFontManager(
      MakeTestFontMgr());

  std::string json = R"json({
    "type": "Paragraph",
    "name": "test_paragraph_with_cursor",
    "id": 210,
    "horizontalAlignment": { "type": "center", "width": 300 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 30, "skewX": 0 },
    "textAlign": "right",
    "textOverflow": "wrap",
    "cursorOffset": 51,
    "texts": [
      { "txt": "Test\n", "fontFamily": "Arial", "fontSize": 20, "fontStyle": { "weight": 100, "slant": "italic" }, "letterSpacing": -1, "wordSpacing": 1, "foregroundColor": { "type": "solid", "color": "#000000" } },
      { "txt": "\n\nA very very very very very very very very long text that should wrap\n", "fontFamily": "Arial", "fontSize": 20, "fontStyle": { "weight": 100, "slant": "italic" }, "letterSpacing": -1, "wordSpacing": 1, "foregroundColor": { "type": "solid", "color": "#000000" } },
      { "txt": "\n\n\n", "fontFamily": "Arial", "fontSize": 20, "fontStyle": { "weight": 100, "slant": "upright" }, "letterSpacing": -1, "wordSpacing": 1, "foregroundColor": { "type": "solid", "color": "#000000" } }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw paragraph with cursor";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_paragraph_with_cursor.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test paragraph with cursor at the end
TEST_F(RenderTest, DrawParagraphWithCursorAtEnd) {
  common_renderer::FontManagerSingleton::getInstance().setFontManager(
      MakeTestFontMgr());

  std::string json = R"json({
    "type": "Paragraph",
    "name": "test_paragraph_with_cursor_at_end",
    "id": 211,
    "horizontalAlignment": { "type": "center", "width": 300 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 30, "skewX": 0 },
    "textAlign": "left",
    "textOverflow": "wrap",
    "cursorOffset": 79,
    "texts": [
      { "txt": "Test\n", "fontFamily": "Arial", "fontSize": 20, "fontStyle": { "weight": 100, "slant": "italic" }, "letterSpacing": -1, "wordSpacing": 1, "foregroundColor": { "type": "solid", "color": "#000000" } },
      { "txt": "\n\nA very very very very very very very very long text that should wrap\n", "fontFamily": "Arial", "fontSize": 20, "fontStyle": { "weight": 100, "slant": "italic" }, "letterSpacing": -1, "wordSpacing": 1, "foregroundColor": { "type": "solid", "color": "#000000" } },
      { "txt": "\n", "fontFamily": "Arial", "fontSize": 20, "fontStyle": { "weight": 100, "slant": "italic" }, "letterSpacing": -1, "wordSpacing": 1, "foregroundColor": { "type": "solid", "color": "#000000" } }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw paragraph with cursor at end";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_paragraph_with_cursor_at_end.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test paragraph with cursor at the beginning
TEST_F(RenderTest, DrawParagraphWithCursorAtStart) {
  common_renderer::FontManagerSingleton::getInstance().setFontManager(
      MakeTestFontMgr());

  std::string json = R"json({
    "type": "Paragraph",
    "name": "test_paragraph_with_cursor_at_start",
    "id": 212,
    "horizontalAlignment": { "type": "center", "width": 300 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": -30, "skewX": 0 },
    "textAlign": "center",
    "cursorOffset": 0,
    "texts": [
      { "txt": "\nTEXT", "fontFamily": "Arial", "fontSize": 50, "fontStyle": { "weight": 700, "slant": "italic" }, "letterSpacing": 3, "wordSpacing": 0, "foregroundColor": { "type": "solid", "color": "#000000" } }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw paragraph with cursor at start";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_paragraph_with_cursor_at_start.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test circle with drop shadow

TEST_F(RenderTest, DrawPath) {
  std::string rectJson = R"json({
    "type": "Layer", "name": "test_rect", "id": 3,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 200 },
    "fill": { "type": "solid", "color": "#000000" }
  })json";
  auto rectScene = skia_parser::parseNode(rectJson, testWidth, testHeight);
  ASSERT_NE(rectScene, nullptr) << "Failed to parse rect";
  bool r1 = common_renderer::draw(testCanvas, rectScene, testWidth, testHeight, false);
  EXPECT_TRUE(r1) << "Failed to draw background rect";

  std::string json = R"json({
    "type": "Path", "name": "test_path", "id": 1,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 200 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "path": { "commands": [
      {"type":"move","x":10,"y":10},{"type":"line","x":190,"y":10},
      {"type":"line","x":190,"y":190},{"type":"line","x":10,"y":190},{"type":"close"}
    ]},
    "fill": { "type": "solid", "color": "#FF0000" }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw Path";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_path.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered Path image does not match expected fixture";
  }
}

// Test drawing a path with stroke
TEST_F(RenderTest, DrawPathWithStroke) {
  std::string json = R"json({
    "type": "Path", "name": "test_path_stroke", "id": 2,
    "horizontalAlignment": { "type": "center", "width": 150 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "path": { "commands": [
      {"type":"move","x":75,"y":10},{"type":"line","x":140,"y":140},
      {"type":"line","x":10,"y":140},{"type":"close"}
    ]},
    "fill": { "type": "solid", "color": "#00FF00" },
    "stroke": { "width": 5, "color": { "type": "solid", "color": "#0000FF" } }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw Path with stroke";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_path_with_stroke.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered Path with stroke image does not match expected fixture";
  }
}

// Test drawing a path with rotation
TEST_F(RenderTest, DrawPathWithRotation) {
  std::string json = R"json({
    "type": "Path", "name": "test_path_rotated", "id": 3,
    "horizontalAlignment": { "type": "center", "width": 100 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 45, "skewX": 0 },
    "path": { "commands": [
      {"type":"move","x":10,"y":10},{"type":"line","x":90,"y":10},
      {"type":"line","x":90,"y":90},{"type":"line","x":10,"y":90},{"type":"close"}
    ]},
    "fill": { "type": "solid", "color": "#800080" },
    "stroke": { "width": 3, "color": { "type": "solid", "color": "#FF00FF" } }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw rotated Path";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_path_rotated.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered rotated Path image does not match expected fixture";
  }
}

// Test drawing a path with trim effect
TEST_F(RenderTest, DrawPathWithTrim) {
  std::string json = R"json({
    "type": "Path", "name": "test_path_trim", "id": 4,
    "horizontalAlignment": { "type": "center", "width": 150 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "path": { "commands": [
      {"type":"move","x":135,"y":75},
      {"type":"cubic","x1":135,"y1":41.880,"x2":108.12,"y2":15,"x":75,"y":15},
      {"type":"cubic","x1":41.880,"y1":15,"x2":15,"y2":41.880,"x":15,"y":75},
      {"type":"cubic","x1":15,"y1":108.12,"x2":41.880,"y2":135,"x":75,"y":135},
      {"type":"cubic","x1":108.12,"y1":135,"x2":135,"y2":108.12,"x":135,"y":75},
      {"type":"close"}
    ]},
    "trim": { "start": 0.25, "end": 0.75 },
    "stroke": { "width": 8, "color": { "type": "solid", "color": "#FF6600" } }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw Path with trim";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_path_with_trim.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered Path with trim image does not match expected fixture";
  }
}

// Test drawing a path with gradient fill
TEST_F(RenderTest, DrawPathWithGradient) {
  std::string json = R"json({
    "type": "Path", "name": "test_path_gradient", "id": 5,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 200 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "path": { "commands": [
      {"type":"move","x":180.0,"y":100.0},
      {"type":"line","x":132.3607,"y":123.5114},{"type":"line","x":124.7214,"y":176.0845},
      {"type":"line","x":87.6393,"y":138.0423},{"type":"line","x":35.2786,"y":147.0228},
      {"type":"line","x":60.0,"y":100.0},{"type":"line","x":35.2786,"y":52.9772},
      {"type":"line","x":87.6393,"y":61.9577},{"type":"line","x":124.7214,"y":23.9155},
      {"type":"line","x":132.3607,"y":76.4886},{"type":"line","x":180.0,"y":100.0},
      {"type":"close"}
    ]},
    "fill": { "type": "gradient", "startColor": "#FF0000", "endColor": "#0000FF", "start": {"x":0,"y":0}, "end": {"x":1,"y":1} }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw Path with gradient";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_path_with_gradient.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered Path with gradient image does not match expected fixture";
  }
}

// Test drawing a path with image fill
TEST_F(RenderTest, DrawPathWithImage) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({
    "type": "Path", "name": "test_path_image", "id": 6,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 300 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "path": { "commands": [
      {"type":"move","x":180.0,"y":100.0},
      {"type":"line","x":132.3607,"y":123.5114},{"type":"line","x":124.7214,"y":176.0845},
      {"type":"line","x":87.6393,"y":138.0423},{"type":"line","x":35.2786,"y":147.0228},
      {"type":"line","x":60.0,"y":100.0},{"type":"line","x":35.2786,"y":52.9772},
      {"type":"line","x":87.6393,"y":61.9577},{"type":"line","x":124.7214,"y":23.9155},
      {"type":"line","x":132.3607,"y":76.4886},{"type":"line","x":180.0,"y":100.0},
      {"type":"close"}
    ]},
    "fill": {"type":"image","imageUrl":")json" + imageUrl + R"json(","fit":"cover"}
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result =
      common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw Path with image";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_path_with_image.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered Path with image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawRootWithPan) {
  // Create a root node with pan transformation
  auto root = std::make_shared<common_renderer::RootNode>();
  root->name = "test_root_pan";
  root->id = 200;

  // Set pan transformation
  root->pan = std::make_tuple(50.0F, 30.0F);  // Pan by (50, 30)
  root->scale = std::make_tuple(1.0F, 1.0F);  // No scaling
  root->origin = std::make_tuple(0.0F, 0.0F); // Origin at (0, 0)

  // Create first child - red circle
  auto circle = std::make_shared<common_renderer::LayerNode>();
  circle->name = "root_circle";
  circle->id = 201;

  // Position circle relative to root
  circle->alignH = common_renderer::HorizontalAlign::LEFT;
  circle->alignV = common_renderer::VerticalAlign::TOP;
  circle->left = {20.0F, false};   // 20px from left
  circle->top = {20.0F, false};    // 20px from top
  circle->width = {80.0F, false};  // 80px wide
  circle->height = {80.0F, false}; // 80px tall

  // Set circle properties
  circle->rotation.cx = 0.5F;
  circle->rotation.cy = 0.5F;
  circle->rotation.z = 0.0F;
  circle->rotation.skewX = 0.0F;
  circle->stroke = std::nullopt;
  circle->clip = common_renderer::OvalClip{true};

  // Set red color for circle
  common_renderer::SolidColor redColor;
  redColor.color = "#FF0000"; // red
  circle->fill = common_renderer::Fill(redColor);

  // Create second child - blue rounded rectangle
  auto rect = std::make_shared<common_renderer::LayerNode>();
  rect->name = "root_rect";
  rect->id = 202;

  // Position rectangle relative to root
  rect->alignH = common_renderer::HorizontalAlign::LEFT;
  rect->alignV = common_renderer::VerticalAlign::TOP;
  rect->left = {120.0F, false};  // 120px from left
  rect->top = {40.0F, false};    // 40px from top
  rect->width = {100.0F, false}; // 100px wide
  rect->height = {60.0F, false}; // 60px tall

  // Set rectangle properties
  rect->rotation.cx = 0.5F;
  rect->rotation.cy = 0.5F;
  rect->rotation.z = 0.0F;
  rect->rotation.skewX = 0.0F;
  common_renderer::Stroke blueThinRectStroke;
  blueThinRectStroke.width = 2.0F;
  common_renderer::SolidColor blueThinRectStrokeColor;
  blueThinRectStrokeColor.color = "#0000FF"; // blue stroke
  blueThinRectStroke.color = common_renderer::Color(blueThinRectStrokeColor);
  rect->stroke = blueThinRectStroke;
  rect->clip = common_renderer::RRectClip{{10.0F, 10.0F, 10.0F, 10.0F}, true};
  rect->clip = common_renderer::RRectClip{{17.5F, 17.5F, 17.5F, 17.5F}, true};

  // Set fill color for rectangle
  common_renderer::SolidColor transparentColor;
  transparentColor.color = "rgba(0, 0, 0, 0)"; // transparent
  rect->fill = common_renderer::Fill(transparentColor);

  // Create third child - green group with nested circle
  auto group = std::make_shared<common_renderer::LayerNode>();
  group->name = "root_group";
  group->id = 203;

  // Position group relative to root
  group->alignH = common_renderer::HorizontalAlign::LEFT;
  group->alignV = common_renderer::VerticalAlign::TOP;
  group->left = {50.0F, false};   // 50px from left
  group->top = {120.0F, false};   // 120px from top
  group->width = {120.0F, false}; // 120px wide
  group->height = {80.0F, false}; // 80px tall

  // Set group properties
  group->rotation.cx = 0.5F;
  group->rotation.cy = 0.5F;
  group->rotation.z = 0.0F;
  group->rotation.skewX = 0.0F;

  // Set group clip properties
  group->clip = common_renderer::RRectClip{{5.0F, 5.0F, 5.0F, 5.0F}, true};

  // Create nested circle inside group
  auto nestedCircle = std::make_shared<common_renderer::LayerNode>();
  nestedCircle->name = "nested_circle";
  nestedCircle->id = 204;

  // Position nested circle relative to group
  nestedCircle->alignH = common_renderer::HorizontalAlign::CENTER;
  nestedCircle->alignV = common_renderer::VerticalAlign::CENTER;
  nestedCircle->width = {60.0F, false};  // 60px wide
  nestedCircle->height = {60.0F, false}; // 60px tall

  // Set nested circle properties
  nestedCircle->rotation.cx = 0.5F;
  nestedCircle->rotation.cy = 0.5F;
  nestedCircle->rotation.z = 45.0F; // 45 degree rotation
  nestedCircle->rotation.skewX = 0.0F;
  common_renderer::Stroke greenNestedCircleStroke1;
  greenNestedCircleStroke1.width = 3.0F;
  common_renderer::SolidColor greenNestedCircleStroke1Color;
  greenNestedCircleStroke1Color.color = "#00FF00"; // green
  greenNestedCircleStroke1.color =
      common_renderer::Color(greenNestedCircleStroke1Color);
  nestedCircle->stroke = greenNestedCircleStroke1;
  nestedCircle->clip = common_renderer::OvalClip{true};

  // Set fill color for nested circle
  nestedCircle->fill = common_renderer::Fill(transparentColor);

  // Add nested circle to group
  group->children.push_back(nestedCircle);

  // Add all children to root
  root->children.push_back(circle);
  root->children.push_back(rect);
  root->children.push_back(group);

  // Draw using common_renderer
  bool result =
      common_renderer::draw(testCanvas, root, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw root node";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_root_with_pan.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test root node rendering with scale transformation
TEST_F(RenderTest, DrawRootWithScale) {
  // Create a root node with scale transformation
  auto root = std::make_shared<common_renderer::RootNode>();
  root->name = "test_root_scale";
  root->id = 300;

  // Set scale transformation
  root->pan = std::make_tuple(0.0F, 0.0F);        // No panning
  root->scale = std::make_tuple(1.5F, 1.2F);      // Scale by (1.5, 1.2)
  root->origin = std::make_tuple(100.0F, 100.0F); // Origin at (100, 100)

  // Create first child - red circle
  auto circle = std::make_shared<common_renderer::LayerNode>();
  circle->name = "root_circle";
  circle->id = 301;

  // Position circle relative to root
  circle->alignH = common_renderer::HorizontalAlign::LEFT;
  circle->alignV = common_renderer::VerticalAlign::TOP;
  circle->left = {20.0F, false};   // 20px from left
  circle->top = {20.0F, false};    // 20px from top
  circle->width = {80.0F, false};  // 80px wide
  circle->height = {80.0F, false}; // 80px tall

  // Set circle properties
  circle->rotation.cx = 0.5F;
  circle->rotation.cy = 0.5F;
  circle->rotation.z = 0.0F;
  circle->rotation.skewX = 0.0F;
  circle->stroke = std::nullopt;
  circle->clip = common_renderer::OvalClip{true};

  // Set red color for circle
  common_renderer::SolidColor redColor;
  redColor.color = "#FF0000"; // red
  circle->fill = common_renderer::Fill(redColor);

  // Create second child - blue rounded rectangle
  auto rect = std::make_shared<common_renderer::LayerNode>();
  rect->name = "root_rect";
  rect->id = 302;

  // Position rectangle relative to root
  rect->alignH = common_renderer::HorizontalAlign::LEFT;
  rect->alignV = common_renderer::VerticalAlign::TOP;
  rect->left = {120.0F, false};  // 120px from left
  rect->top = {40.0F, false};    // 40px from top
  rect->width = {100.0F, false}; // 100px wide
  rect->height = {60.0F, false}; // 60px tall

  // Set rectangle properties
  rect->rotation.cx = 0.5F;
  rect->rotation.cy = 0.5F;
  rect->rotation.z = 0.0F;
  rect->rotation.skewX = 0.0F;
  common_renderer::Stroke blueThinRectStroke2;
  blueThinRectStroke2.width = 2.0F;
  common_renderer::SolidColor blueThinRectStroke2Color;
  blueThinRectStroke2Color.color = "#0000FF"; // blue
  blueThinRectStroke2.color = common_renderer::Color(blueThinRectStroke2Color);
  rect->stroke = blueThinRectStroke2;
  rect->clip = common_renderer::RRectClip{{10.0F, 10.0F, 10.0F, 10.0F}, true};
  rect->clip = common_renderer::RRectClip{{17.5F, 17.5F, 17.5F, 17.5F}, true};

  // Set fill color for rectangle
  common_renderer::SolidColor transparentColor;
  transparentColor.color = "rgba(0,0,0,0)"; // transparent
  rect->fill = common_renderer::Fill(transparentColor);

  // Create third child - green group with nested circle
  auto group = std::make_shared<common_renderer::LayerNode>();
  group->name = "root_group";
  group->id = 303;

  // Position group relative to root
  group->alignH = common_renderer::HorizontalAlign::LEFT;
  group->alignV = common_renderer::VerticalAlign::TOP;
  group->left = {50.0F, false};   // 50px from left
  group->top = {120.0F, false};   // 120px from top
  group->width = {120.0F, false}; // 120px wide
  group->height = {80.0F, false}; // 80px tall

  // Set group properties
  group->rotation.cx = 0.5F;
  group->rotation.cy = 0.5F;
  group->rotation.z = 0.0F;
  group->rotation.skewX = 0.0F;

  // Set group clip properties
  group->clip = common_renderer::RRectClip{{5.0F, 5.0F, 5.0F, 5.0F}, true};

  // Create nested circle inside group
  auto nestedCircle = std::make_shared<common_renderer::LayerNode>();
  nestedCircle->name = "nested_circle";
  nestedCircle->id = 304;

  // Position nested circle relative to group
  nestedCircle->alignH = common_renderer::HorizontalAlign::CENTER;
  nestedCircle->alignV = common_renderer::VerticalAlign::CENTER;
  nestedCircle->width = {60.0F, false};  // 60px wide
  nestedCircle->height = {60.0F, false}; // 60px tall

  // Set nested circle properties
  nestedCircle->rotation.cx = 0.5F;
  nestedCircle->rotation.cy = 0.5F;
  nestedCircle->rotation.z = 45.0F; // 45 degree rotation
  nestedCircle->rotation.skewX = 0.0F;
  common_renderer::Stroke greenNestedCircleStroke2;
  greenNestedCircleStroke2.width = 3.0F;
  common_renderer::SolidColor greenNestedCircleStroke2Color;
  greenNestedCircleStroke2Color.color = "#00FF00"; // green
  greenNestedCircleStroke2.color =
      common_renderer::Color(greenNestedCircleStroke2Color);
  nestedCircle->stroke = greenNestedCircleStroke2;
  nestedCircle->clip = common_renderer::OvalClip{true};

  // Set fill color for nested circle
  nestedCircle->fill = common_renderer::Fill(transparentColor);

  // Add nested circle to group
  group->children.push_back(nestedCircle);

  // Add all children to root
  root->children.push_back(circle);
  root->children.push_back(rect);
  root->children.push_back(group);

  // Draw using common_renderer
  bool result =
      common_renderer::draw(testCanvas, root, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw root node with scale";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_root_with_scale.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawRoundedRectCenter) {
  std::string json = R"json({
    "type": "Layer", "name": "test_rounded_rect", "id": 3,
    "horizontalAlignment": { "type": "center", "width": 150 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "rotation": { "cx": 0, "cy": 0, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "clip": { "type": "rrect", "enable": true, "r": [20, 20, 20, 20] }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw rounded rectangle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_rounded_rect_center.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test rounded rectangle with stroke
TEST_F(RenderTest, DrawRoundedRectWithTrimAndStroke) {
  std::string json = R"json({
    "type": "Layer", "name": "test_rounded_rect_stroke", "id": 4,
    "horizontalAlignment": { "type": "left", "left": 50, "width": 120 },
    "verticalAlignment": { "type": "top", "top": 50, "height": 80 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "rgba(0, 0, 0, 0)" },
    "clip": { "type": "rrect", "enable": true, "r": [20, 20, 20, 20] },
    "trim": { "start": 0.2, "end": 0.8 },
    "stroke": { "width": 4, "color": { "type": "solid", "color": "#0000FF" } }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw rounded rectangle with stroke";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile =
      "./fixture/draw_rounded_rect_with_trim_and_stroke.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test rounded rectangle positioning - top left
TEST_F(RenderTest, DrawRoundedRectTopLeft) {
  std::string json = R"json({
    "type": "Layer", "name": "test_rounded_rect", "id": 3,
    "horizontalAlignment": { "type": "left", "left": 20, "width": 100 },
    "verticalAlignment": { "type": "top", "top": 20, "height": 60 },
    "rotation": { "cx": 0, "cy": 0, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "clip": { "type": "rrect", "enable": true, "r": [20, 20, 20, 20] }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw positioned rounded rectangle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_rounded_rect_top_left.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile))
        << "Failed to save positioned rounded rectangle";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test rounded rectangle positioning - bottom right
TEST_F(RenderTest, DrawRoundedRectBottomRight) {
  std::string json = R"json({
    "type": "Layer", "name": "test_rounded_rect", "id": 3,
    "horizontalAlignment": { "type": "right", "right": 20, "width": 100 },
    "verticalAlignment": { "type": "bottom", "bottom": 20, "height": 60 },
    "rotation": { "cx": 0, "cy": 0, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "clip": { "type": "rrect", "enable": true, "r": [20, 20, 20, 20] }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw positioned rounded rectangle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_rounded_rect_bottom_right.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile))
        << "Failed to save positioned rounded rectangle";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test rounded rectangle with trim (partial rectangle)
TEST_F(RenderTest, DrawRoundedRectWithTrim) {
  std::string json = R"json({
    "type": "Layer", "name": "test_rounded_rect", "id": 3,
    "horizontalAlignment": { "type": "center", "width": 150 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "rotation": { "cx": 0, "cy": 0, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "clip": { "type": "rrect", "enable": true, "r": [20, 20, 20, 20] },
    "trim": { "start": 0, "end": 0.5 }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw trimmed rounded rectangle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_rounded_rect_with_trim.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile))
        << "Failed to save trimmed rounded rectangle";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test rounded rectangle with rotation
TEST_F(RenderTest, DrawRoundedRectWithRotation) {
  std::string json = R"json({
    "type": "Layer", "name": "test_rounded_rect", "id": 3,
    "horizontalAlignment": { "type": "center", "width": 150 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "rotation": { "cx": 1, "cy": 1, "z": 30, "skewX": 0 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "clip": { "type": "rrect", "enable": true, "r": [20, 20, 20, 20] }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw rotated rounded rectangle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_rounded_rect_with_rotation.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile))
        << "Failed to save rotated rounded rectangle";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test rounded rectangle with different corner radii
TEST_F(RenderTest, DrawRoundedRectDifferentCornerRadii) {
  std::string json = R"json({
    "type": "Layer", "name": "test_rounded_rect", "id": 3,
    "horizontalAlignment": { "type": "center", "width": 150 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "rotation": { "cx": 0, "cy": 0, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "clip": { "type": "rrect", "enable": true, "r": [300, 30, 0, 100] }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw rounded rectangle with different corner radii";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile =
      "./fixture/draw_rounded_rect_different_corner_radii.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile))
        << "Failed to save rounded rectangle with different corner radii";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test rounded rectangle with no corner radii (regular rectangle)
TEST_F(RenderTest, DrawRoundedRectNoCornerRadii) {
  std::string json = R"json({
    "type": "Layer", "name": "test_rounded_rect", "id": 3,
    "horizontalAlignment": { "type": "center", "width": 150 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "rotation": { "cx": 0, "cy": 0, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "clip": { "type": "rrect", "enable": true, "r": [0, 0, 0, 0] }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw rounded rectangle with no corner radii";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_rounded_rect_no_corner_radii.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile))
        << "Failed to save rounded rectangle with no corner radii";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawRoundedRectWithGradient) {
  std::string json = R"json({
    "type": "Layer", "name": "gradient_rect", "id": 101,
    "horizontalAlignment": { "type": "center", "width": 150 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "fill": { "type": "gradient", "startColor": "#00FF00", "endColor": "#FF0000",
              "start": {"x": 0, "y": 0}, "end": {"x": 1, "y": 1} },
    "clip": { "type": "rrect", "enable": true, "r": [20, 20, 20, 20] }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw gradient rounded rectangle";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_rounded_rect_with_gradient.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test rounded rectangle with gradient and stroke
TEST_F(RenderTest, DrawRoundedRectWithGradientAndStroke) {
  std::string json = R"json({
    "type": "Layer", "name": "gradient_rect_stroke", "id": 104,
    "horizontalAlignment": { "type": "center", "width": 120 },
    "verticalAlignment": { "type": "center", "height": 80 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "rgba(0, 0, 0, 0)" },
    "clip": { "type": "rrect", "enable": true, "r": [10, 10, 10, 10] },
    "stroke": { "width": 4, "color": { "type": "gradient", "startColor": "#FFA500", "endColor": "#800080",
                "start": {"x": 0, "y": 0}, "end": {"x": 1, "y": 1} } }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw gradient rounded rectangle with stroke";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile =
      "./fixture/draw_rounded_rect_with_gradient_and_stroke.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}
TEST_F(RenderTest, DrawRoundedRectFillAndStroke) {
  std::string json = R"json({
    "type": "Layer", "name": "test_rect_fill_stroke", "id": 2,
    "horizontalAlignment": { "type": "center", "width": 120 },
    "verticalAlignment": { "type": "center", "height": 80 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#00FF00" },
    "clip": { "type": "rrect", "enable": true, "r": [15, 15, 15, 15] },
    "stroke": { "width": 4, "color": { "type": "solid", "color": "#800080" } }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw rounded rectangle with fill and stroke";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_rounded_rect_fill_and_stroke.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawScreen) {
  std::string json = R"json({
    "type": "Screen", "name": "test_screen", "id": 100,
    "horizontalAlignment": { "type": "center", "width": 300 },
    "verticalAlignment": { "type": "center", "height": 200 },
    "fill": { "type": "solid", "color": "#008080" },
    "clip": { "type": "rrect", "enable": false, "r": [10, 10, 10, 10] },
    "children": [
      {
        "type": "Layer", "name": "screen_circle", "id": 101,
        "horizontalAlignment": { "type": "left", "left": 50, "width": 60 },
        "verticalAlignment": { "type": "top", "top": 50, "height": 60 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#FF0000" },
        "clip": { "type": "oval", "enable": true }
      },
      {
        "type": "Layer", "name": "screen_rect", "id": 102,
        "horizontalAlignment": { "type": "right", "right": 50, "width": 80 },
        "verticalAlignment": { "type": "bottom", "bottom": 50, "height": 50 },
        "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
        "fill": { "type": "solid", "color": "#0000FF" },
        "clip": { "type": "rrect", "enable": true, "r": [10, 10, 10, 10] }
      }
    ]
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw screen";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_screen.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawScreenWithImage) {
  std::string imageUrl = loadImageAsDataUrl("src/test_image.jpg");
  std::string json = R"json({
    "type": "Screen", "name": "test_screen_with_image", "id": 200,
    "horizontalAlignment": { "type": "center", "width": 300 },
    "verticalAlignment": { "type": "center", "height": 200 },
    "fill": { "type": "image", "imageUrl": ")json" + imageUrl + R"json(", "fit": "cover" },
    "clip": { "type": "rrect", "enable": true, "r": [40, 40, 40, 40] },
    "stroke": { "width": 4, "color": { "type": "solid", "color": "#FF0000" } }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw screen with image";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_screen_with_image.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

TEST_F(RenderTest, MakeSnapshotNullSurface) {
  uint8_t *snapshotPtr = nullptr;

  size_t size = common_renderer::makeSnapshot(nullptr, &snapshotPtr);

  EXPECT_EQ(size, 0) << "Should return 0 for null surface";
  EXPECT_EQ(snapshotPtr, nullptr) << "Should set out_ptr to null on failure";
}

// Test make_snapshot with null out pointer
TEST_F(RenderTest, MakeSnapshotNullOutPtr) {
  size_t size =
      common_renderer::makeSnapshot(testSurface.get(), nullptr);

  EXPECT_EQ(size, 0) << "Should return 0 for null out pointer";
}

// Test make_snapshot with valid single circle
TEST_F(RenderTest, MakeSnapshotValidCircle) {
  auto circle = createRedCircle();

  // Draw the circle first
  bool drawResult =
      common_renderer::draw(testCanvas, circle, testWidth, testHeight);
  EXPECT_TRUE(drawResult) << "Failed to draw circle";

  uint8_t *snapshotPtr = nullptr;

  // Take snapshot
  size_t size =
      common_renderer::makeSnapshot(testSurface.get(), &snapshotPtr);

  EXPECT_GT(size, 0) << "Should return size > 0 for valid snapshot";
  EXPECT_NE(snapshotPtr, nullptr) << "Should set out_ptr to valid memory";

  if (snapshotPtr != nullptr && size > 0) {
    // Verify it's valid PNG data by checking magic bytes
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    EXPECT_EQ(snapshotPtr[0], 0x89) << "Should be valid PNG (magic byte 1)";
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    EXPECT_EQ(snapshotPtr[1], 0x50) << "Should be valid PNG (magic byte 2)";
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    EXPECT_EQ(snapshotPtr[2], 0x4E) << "Should be valid PNG (magic byte 3)";
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    EXPECT_EQ(snapshotPtr[3], 0x47) << "Should be valid PNG (magic byte 4)";

    // Decode snapshot to get pixel data
    int snapshotWidth = 0, snapshotHeight = 0;
    auto snapshotPixels = decodePNGBytesToPixels(snapshotPtr, size,
                                                 snapshotWidth, snapshotHeight);
    EXPECT_FALSE(snapshotPixels.empty()) << "Failed to decode snapshot pixels";

    // Check if fixture exists
    std::string fixtureFile = "./fixture/make_snapshot_valid_circle.png";
    if (!std::filesystem::exists(fixtureFile)) {
      // Fixture doesn't exist, save snapshot to fixture
      EXPECT_TRUE(saveSnapshotToPNG(snapshotPtr, size, fixtureFile))
          << "Failed to save snapshot image";
      // Just save, don't fail on first run
      // Clean up
      freeSnapshot(snapshotPtr);
      return;
    }

    // Load fixture and compare
    auto fixturePixels = loadPNGPixels(fixtureFile);
    EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

    if (!fixturePixels.empty() && !snapshotPixels.empty()) {
      EXPECT_TRUE(compareImages(snapshotPixels, fixturePixels, snapshotWidth,
                                snapshotHeight, 0.0F))
          << "Snapshot does not match expected fixture";
    }

    // Clean up
    freeSnapshot(snapshotPtr);
  }
}

// Test make_snapshot with rotated element
TEST_F(RenderTest, MakeSnapshotRotatedElement) {
  auto circle = createRedCircle();
  // cx and cy are percentages relative to width/height, default to 0.5 (center)
  circle->rotation.cx = 0.5F;
  circle->rotation.cy = 0.5F;
  circle->rotation.z = 45.0F; // 45 degree rotation

  // Draw the circle first
  bool drawResult =
      common_renderer::draw(testCanvas, circle, testWidth, testHeight);
  EXPECT_TRUE(drawResult) << "Failed to draw rotated circle";

  uint8_t *snapshotPtr = nullptr;

  // Take snapshot
  size_t size =
      common_renderer::makeSnapshot(testSurface.get(), &snapshotPtr);

  EXPECT_GT(size, 0) << "Should return size > 0 for rotated element";
  EXPECT_NE(snapshotPtr, nullptr)
      << "Should set out_ptr to valid memory for rotated element";

  if (snapshotPtr != nullptr) {
    // Check PNG magic bytes
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    EXPECT_EQ(snapshotPtr[0], 0x89) << "Should be valid PNG";

    // Decode snapshot to get pixel data
    int snapshotWidth = 0, snapshotHeight = 0;
    auto snapshotPixels = decodePNGBytesToPixels(snapshotPtr, size,
                                                 snapshotWidth, snapshotHeight);
    EXPECT_FALSE(snapshotPixels.empty()) << "Failed to decode snapshot pixels";

    // Check if fixture exists
    std::string fixtureFile = "./fixture/make_snapshot_rotated_element.png";
    if (!std::filesystem::exists(fixtureFile)) {
      EXPECT_TRUE(saveSnapshotToPNG(snapshotPtr, size, fixtureFile))
          << "Failed to save snapshot image";
      freeSnapshot(snapshotPtr);
      return;
    }

    // Load fixture and compare
    auto fixturePixels = loadPNGPixels(fixtureFile);
    EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

    if (!fixturePixels.empty() && !snapshotPixels.empty()) {
      EXPECT_TRUE(compareImages(snapshotPixels, fixturePixels, snapshotWidth,
                                snapshotHeight, 0.0F))
          << "Snapshot does not match expected fixture";
    }

    // Clean up
    freeSnapshot(snapshotPtr);
  }
}

// Test make_snapshot with group node (snapshot whole group)
TEST_F(RenderTest, MakeSnapshotGroup) {
  auto group = createGroupWithChildren();

  // Draw the group first
  bool drawResult =
      common_renderer::draw(testCanvas, group, testWidth, testHeight);
  EXPECT_TRUE(drawResult) << "Failed to draw group";

  uint8_t *snapshotPtr = nullptr;

  // Take snapshot of the group
  size_t size =
      common_renderer::makeSnapshot(testSurface.get(), &snapshotPtr);

  EXPECT_GT(size, 0) << "Should return size > 0 for group snapshot";
  EXPECT_NE(snapshotPtr, nullptr)
      << "Should set out_ptr to valid memory for group";

  if (snapshotPtr != nullptr) {
    // Check PNG magic bytes
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    EXPECT_EQ(snapshotPtr[0], 0x89) << "Should be valid PNG";

    // Decode snapshot to get pixel data
    int snapshotWidth = 0, snapshotHeight = 0;
    auto snapshotPixels = decodePNGBytesToPixels(snapshotPtr, size,
                                                 snapshotWidth, snapshotHeight);
    EXPECT_FALSE(snapshotPixels.empty()) << "Failed to decode snapshot pixels";

    // Check if fixture exists
    std::string fixtureFile = "./fixture/make_snapshot_group.png";
    if (!std::filesystem::exists(fixtureFile)) {
      EXPECT_TRUE(saveSnapshotToPNG(snapshotPtr, size, fixtureFile))
          << "Failed to save snapshot image";
      freeSnapshot(snapshotPtr);
      return;
    }

    // Load fixture and compare
    auto fixturePixels = loadPNGPixels(fixtureFile);
    EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

    if (!fixturePixels.empty() && !snapshotPixels.empty()) {
      EXPECT_TRUE(compareImages(snapshotPixels, fixturePixels, snapshotWidth,
                                snapshotHeight, 0.0F))
          << "Snapshot does not match expected fixture";
    }

    // Clean up
    freeSnapshot(snapshotPtr);
  }
}

// Test make_snapshot with root node transformation (pan)
TEST_F(RenderTest, MakeSnapshotWithRootTransformation) {
  auto root = std::make_shared<common_renderer::RootNode>();
  root->name = "test_root";
  root->id = 200;

  // Set pan transformation
  root->pan = std::make_tuple(50.0F, 30.0F);
  root->scale = std::make_tuple(3.0F, 3.0F);
  root->origin = std::make_tuple(20.0F, 20.0F);

  // Add a circle as child
  auto circle = createRedCircle();
  circle->id = 201;
  circle->alignH = common_renderer::HorizontalAlign::LEFT;
  circle->alignV = common_renderer::VerticalAlign::TOP;
  circle->left = {20.0F, false};
  circle->top = {20.0F, false};
  circle->width = {80.0F, false};
  circle->height = {80.0F, false};

  root->children.push_back(circle);

  // Draw the root
  bool drawResult =
      common_renderer::draw(testCanvas, root, testWidth, testHeight);
  EXPECT_TRUE(drawResult) << "Failed to draw root";

  uint8_t *snapshotPtr = nullptr;

  // Take snapshot
  size_t size =
      common_renderer::makeSnapshot(testSurface.get(), &snapshotPtr);

  EXPECT_GT(size, 0) << "Should return size > 0 with root transformation";
  EXPECT_NE(snapshotPtr, nullptr)
      << "Should set out_ptr to valid memory with root transformation";

  if (snapshotPtr != nullptr) {
    // Check PNG magic bytes
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    EXPECT_EQ(snapshotPtr[0], 0x89) << "Should be valid PNG";

    // Decode snapshot to get pixel data
    int snapshotWidth = 0, snapshotHeight = 0;
    auto snapshotPixels = decodePNGBytesToPixels(snapshotPtr, size,
                                                 snapshotWidth, snapshotHeight);
    EXPECT_FALSE(snapshotPixels.empty()) << "Failed to decode snapshot pixels";

    // Check if fixture exists
    std::string fixtureFile =
        "./fixture/make_snapshot_with_root_transformation.png";
    if (!std::filesystem::exists(fixtureFile)) {
      EXPECT_TRUE(saveSnapshotToPNG(snapshotPtr, size, fixtureFile))
          << "Failed to save snapshot image";
      freeSnapshot(snapshotPtr);
      return;
    }

    // Load fixture and compare
    auto fixturePixels = loadPNGPixels(fixtureFile);
    EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

    if (!fixturePixels.empty() && !snapshotPixels.empty()) {
      EXPECT_TRUE(compareImages(snapshotPixels, fixturePixels, snapshotWidth,
                                snapshotHeight, 0.0F))
          << "Snapshot does not match expected fixture";
    }

    // Clean up
    freeSnapshot(snapshotPtr);
  }
}

// Test make_snapshot multiple times on same surface
TEST_F(RenderTest, MakeSnapshotMultipleTimes) {
  auto group = createGroupWithChildren();

  // Draw the group
  bool drawResult =
      common_renderer::draw(testCanvas, group, testWidth, testHeight);
  EXPECT_TRUE(drawResult) << "Failed to draw group";

  // Take first snapshot
  uint8_t *snapshotPtr1 = nullptr;
  size_t size1 =
      common_renderer::makeSnapshot(testSurface.get(), &snapshotPtr1);
  EXPECT_GT(size1, 0) << "First snapshot should succeed";

  // Take second snapshot
  uint8_t *snapshotPtr2 = nullptr;
  size_t size2 =
      common_renderer::makeSnapshot(testSurface.get(), &snapshotPtr2);
  EXPECT_GT(size2, 0) << "Second snapshot should succeed";

  // Take third snapshot
  uint8_t *snapshotPtr3 = nullptr;
  size_t size3 =
      common_renderer::makeSnapshot(testSurface.get(), &snapshotPtr3);
  EXPECT_GT(size3, 0) << "Third snapshot should succeed";

  // All snapshots should be valid
  if (snapshotPtr1 != nullptr && snapshotPtr2 != nullptr &&
      snapshotPtr3 != nullptr) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    EXPECT_EQ(snapshotPtr1[0], 0x89) << "First snapshot should be valid PNG";
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    EXPECT_EQ(snapshotPtr2[0], 0x89) << "Second snapshot should be valid PNG";
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    EXPECT_EQ(snapshotPtr3[0], 0x89) << "Third snapshot should be valid PNG";
  }

  // Clean up
  freeSnapshot(snapshotPtr1);
  freeSnapshot(snapshotPtr2);
  freeSnapshot(snapshotPtr3);
}

// Test make_snapshot with element that extends beyond canvas bounds
TEST_F(RenderTest, MakeSnapshotBeyondBounds) {
  auto circle = createRedCircle();

  // Position circle so it extends beyond canvas bounds
  circle->alignH = common_renderer::HorizontalAlign::LEFT;
  circle->alignV = common_renderer::VerticalAlign::TOP;
  circle->left = {350.0F, false}; // Near right edge
  circle->top = {250.0F, false};  // Near bottom edge
  circle->width = {100.0F, false};
  circle->height = {100.0F, false};

  // Draw the circle
  bool drawResult =
      common_renderer::draw(testCanvas, circle, testWidth, testHeight);
  EXPECT_TRUE(drawResult) << "Failed to draw circle";

  uint8_t *snapshotPtr = nullptr;

  // Take snapshot - should handle out-of-bounds gracefully
  size_t size =
      common_renderer::makeSnapshot(testSurface.get(), &snapshotPtr);

  // Function should still succeed (bounds will be clamped)
  EXPECT_GT(size, 0) << "Should handle out-of-bounds element";
  EXPECT_NE(snapshotPtr, nullptr)
      << "Should set out_ptr to valid memory even for out-of-bounds";

  if (snapshotPtr != nullptr) {
    // Check PNG magic bytes
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    EXPECT_EQ(snapshotPtr[0], 0x89) << "Should be valid PNG";

    // Clean up
    freeSnapshot(snapshotPtr);
  }
}

TEST_F(RenderTest, DrawSVG) {
  std::string rectJson = R"json({
    "type": "Layer", "name": "test_rect", "id": 3,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 200 },
    "fill": { "type": "solid", "color": "#000000" }
  })json";
  auto rectScene = skia_parser::parseNode(rectJson, testWidth, testHeight);
  ASSERT_NE(rectScene, nullptr) << "Failed to parse rect scene";
  bool r1 = common_renderer::draw(testCanvas, rectScene, testWidth, testHeight, false);
  EXPECT_TRUE(r1) << "Failed to draw rect background";

  std::string svgJson = R"json({
    "type": "SVG", "name": "test_svg", "id": 1,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 200 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 0, "skewX": 0 },
    "svg": "<svg width=\"100\" height=\"100\" xmlns=\"http://www.w3.org/2000/svg\">\n  <rect x=\"0\" y=\"0\" width=\"100\" height=\"100\" fill=\"none\" stroke=\"red\" stroke-width=\"4\"/>\n</svg>"
  })json";
  auto scene = skia_parser::parseNode(svgJson, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse SVG scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw SVG";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_svg.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered SVG image does not match expected fixture";
  }
}

TEST_F(RenderTest, DrawSVGWithRotation) {
  std::string rectJson = R"json({
    "type": "Layer", "name": "test_rect", "id": 3,
    "horizontalAlignment": { "type": "center", "width": 100 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "fill": { "type": "solid", "color": "#000000" }
  })json";
  auto rectScene = skia_parser::parseNode(rectJson, testWidth, testHeight);
  ASSERT_NE(rectScene, nullptr) << "Failed to parse rect scene";
  bool r1 = common_renderer::draw(testCanvas, rectScene, testWidth, testHeight, false);
  EXPECT_TRUE(r1) << "Failed to draw rect background";

  std::string svgJson = R"json({
    "type": "SVG", "name": "test_svg_rotated", "id": 2,
    "horizontalAlignment": { "type": "center", "width": 100 },
    "verticalAlignment": { "type": "center", "height": 100 },
    "rotation": { "cx": 0.5, "cy": 0.5, "z": 45, "skewX": 0 },
    "svg": "<svg width=\"100\" height=\"100\" xmlns=\"http://www.w3.org/2000/svg\">\n  <rect x=\"10\" y=\"10\" width=\"80\" height=\"80\" fill=\"green\" stroke=\"purple\" stroke-width=\"2\"/>\n</svg>"
  })json";
  auto scene = skia_parser::parseNode(svgJson, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse SVG scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw rotated SVG";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_svg_rotated.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered rotated SVG image does not match expected fixture";
  }
}



// Helper function to free memory allocated by make_snapshot
void freeSnapshot(uint8_t *ptr) {
  if (ptr) {
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc,hicpp-no-malloc,cppcoreguidelines-owning-memory)
    std::free(ptr);
  }
}

// Helper function to decode PNG bytes to get pixel data
std::vector<uint32_t> decodePNGBytesToPixels(const uint8_t *data, size_t size,
                                             int &width, int &height) {
  std::vector<uint32_t> pixels;

  if (!data || size == 0) {
    return pixels;
  }

  // Create SkData from bytes
  sk_sp<SkData> skData = SkData::MakeWithoutCopy(data, size);
  sk_sp<SkImage> image = SkImages::DeferredFromEncodedData(skData);

  if (!image) {
    std::cerr << "Failed to decode PNG bytes" << std::endl;
    return pixels;
  }

  // Get image dimensions
  width = image->width();
  height = image->height();

  // Get pixel data
  SkPixmap pixmap;
  if (!image->peekPixels(&pixmap)) {
    // peekPixels usually fails, so use readPixels as fallback
    SkImageInfo info = image->imageInfo();
    pixels.resize(info.width() * info.height());

    if (!image->readPixels(info, pixels.data(), info.minRowBytes(), 0, 0)) {
      std::cerr << "Failed to read pixels from decoded PNG" << std::endl;
      pixels.clear();
    }
  } else {
    // Copy pixels to vector
    const auto *pixelPtr = static_cast<const uint32_t *>(pixmap.addr());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    pixels.assign(pixelPtr, pixelPtr + (pixmap.width() * pixmap.height()));
  }

  return pixels;
}

// Helper function to save snapshot PNG bytes to file
bool saveSnapshotToPNG(const uint8_t *data, size_t size,
                       const std::string &filename) {
  if (!data || size == 0) {
    return false;
  }

  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Failed to open file for writing: " << filename << std::endl;
    return false;
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  file.write(reinterpret_cast<const char *>(data),
             static_cast<std::streamsize>(size));
  file.close();

  return true;
}

// Test drawTransition MoveIn Up
TEST_F(RenderTest, DrawTransitionMoveInUp) {
  // Create transition images using test_scenes helper
  sk_sp<SkImage> sourceImage;
  sk_sp<SkImage> targetImage;
  bool imagesCreated = createTransitionImages(
      testWidth, testHeight, sourceImage, targetImage);
  ASSERT_TRUE(imagesCreated) << "Failed to create transition images";
  ASSERT_NE(sourceImage, nullptr) << "Source image is null";
  ASSERT_NE(targetImage, nullptr) << "Target image is null";

  // Clear the test canvas
  testCanvas->clear(SK_ColorTRANSPARENT);

  // Draw transition with progress 0.1, direction up (0, -1)
  float progress = 0.1F;
  common_renderer::TransitionType animationType =
      common_renderer::TransitionType::MOVEIN;
  float directionX = 0.0F;
  float directionY = -1.0F;
  float dpr = 1.0F; // Test with DPR of 1.0 (no scaling)
  bool result = common_renderer::drawTransition(
      testCanvas, sourceImage, targetImage, progress, animationType, directionX,
      directionY, testWidth, testHeight, dpr);
  EXPECT_TRUE(result) << "Failed to draw transition";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_transition_move_in_up.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test drawTransition MoveIn Down
TEST_F(RenderTest, DrawTransitionMoveInDown) {
  // Create transition images using test_scenes helper
  sk_sp<SkImage> sourceImage;
  sk_sp<SkImage> targetImage;
  bool imagesCreated = createTransitionImages(
      testWidth, testHeight, sourceImage, targetImage);
  ASSERT_TRUE(imagesCreated) << "Failed to create transition images";
  ASSERT_NE(sourceImage, nullptr) << "Source image is null";
  ASSERT_NE(targetImage, nullptr) << "Target image is null";

  // Clear the test canvas
  testCanvas->clear(SK_ColorTRANSPARENT);

  // Draw transition with progress 0.1, direction down (0, 1)
  float progress = 0.1F;
  common_renderer::TransitionType animationType =
      common_renderer::TransitionType::MOVEIN;
  float directionX = 0.0F;
  float directionY = 1.0F;
  float dpr = 1.0F; // Test with DPR of 1.0 (no scaling)
  bool result = common_renderer::drawTransition(
      testCanvas, sourceImage, targetImage, progress, animationType, directionX,
      directionY, testWidth, testHeight, dpr);
  EXPECT_TRUE(result) << "Failed to draw transition";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_transition_move_in_down.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test drawTransition MoveIn Left
TEST_F(RenderTest, DrawTransitionMoveInLeft) {
  // Create transition images using test_scenes helper
  sk_sp<SkImage> sourceImage;
  sk_sp<SkImage> targetImage;
  bool imagesCreated = createTransitionImages(
      testWidth, testHeight, sourceImage, targetImage);
  ASSERT_TRUE(imagesCreated) << "Failed to create transition images";
  ASSERT_NE(sourceImage, nullptr) << "Source image is null";
  ASSERT_NE(targetImage, nullptr) << "Target image is null";

  // Clear the test canvas
  testCanvas->clear(SK_ColorTRANSPARENT);

  // Draw transition with progress 0.1, direction left (-1, 0)
  float progress = 0.1F;
  common_renderer::TransitionType animationType =
      common_renderer::TransitionType::MOVEIN;
  float directionX = -1.0F;
  float directionY = 0.0F;
  float dpr = 1.0F; // Test with DPR of 1.0 (no scaling)
  bool result = common_renderer::drawTransition(
      testCanvas, sourceImage, targetImage, progress, animationType, directionX,
      directionY, testWidth, testHeight, dpr);
  EXPECT_TRUE(result) << "Failed to draw transition";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_transition_move_in_left.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test drawTransition MoveIn Right
TEST_F(RenderTest, DrawTransitionMoveInRight) {
  // Create transition images using test_scenes helper
  sk_sp<SkImage> sourceImage;
  sk_sp<SkImage> targetImage;
  bool imagesCreated = createTransitionImages(
      testWidth, testHeight, sourceImage, targetImage);
  ASSERT_TRUE(imagesCreated) << "Failed to create transition images";
  ASSERT_NE(sourceImage, nullptr) << "Source image is null";
  ASSERT_NE(targetImage, nullptr) << "Target image is null";

  // Clear the test canvas
  testCanvas->clear(SK_ColorTRANSPARENT);

  // Draw transition with progress 0.1, direction right (1, 0)
  float progress = 0.1F;
  common_renderer::TransitionType animationType =
      common_renderer::TransitionType::MOVEIN;
  float directionX = 1.0F;
  float directionY = 0.0F;
  float dpr = 1.0F; // Test with DPR of 1.0 (no scaling)
  bool result = common_renderer::drawTransition(
      testCanvas, sourceImage, targetImage, progress, animationType, directionX,
      directionY, testWidth, testHeight, dpr);
  EXPECT_TRUE(result) << "Failed to draw transition";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_transition_move_in_right.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test drawTransition MoveOut Up
TEST_F(RenderTest, DrawTransitionMoveOutUp) {
  // Create transition images using test_scenes helper
  sk_sp<SkImage> sourceImage;
  sk_sp<SkImage> targetImage;
  bool imagesCreated = createTransitionImages(
      testWidth, testHeight, sourceImage, targetImage);
  ASSERT_TRUE(imagesCreated) << "Failed to create transition images";
  ASSERT_NE(sourceImage, nullptr) << "Source image is null";
  ASSERT_NE(targetImage, nullptr) << "Target image is null";

  // Clear the test canvas
  testCanvas->clear(SK_ColorTRANSPARENT);

  // Draw transition with progress 0.1, direction up (0, -1)
  float progress = 0.1F;
  common_renderer::TransitionType animationType =
      common_renderer::TransitionType::MOVEOUT;
  float directionX = 0.0F;
  float directionY = -1.0F;
  float dpr = 1.0F; // Test with DPR of 1.0 (no scaling)
  bool result = common_renderer::drawTransition(
      testCanvas, sourceImage, targetImage, progress, animationType, directionX,
      directionY, testWidth, testHeight, dpr);
  EXPECT_TRUE(result) << "Failed to draw transition";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_transition_move_out_up.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test drawTransition MoveOut Down
TEST_F(RenderTest, DrawTransitionMoveOutDown) {
  // Create transition images using test_scenes helper
  sk_sp<SkImage> sourceImage;
  sk_sp<SkImage> targetImage;
  bool imagesCreated = createTransitionImages(
      testWidth, testHeight, sourceImage, targetImage);
  ASSERT_TRUE(imagesCreated) << "Failed to create transition images";
  ASSERT_NE(sourceImage, nullptr) << "Source image is null";
  ASSERT_NE(targetImage, nullptr) << "Target image is null";

  // Clear the test canvas
  testCanvas->clear(SK_ColorTRANSPARENT);

  // Draw transition with progress 0.1, direction down (0, 1)
  float progress = 0.1F;
  common_renderer::TransitionType animationType =
      common_renderer::TransitionType::MOVEOUT;
  float directionX = 0.0F;
  float directionY = 1.0F;
  float dpr = 1.0F; // Test with DPR of 1.0 (no scaling)
  bool result = common_renderer::drawTransition(
      testCanvas, sourceImage, targetImage, progress, animationType, directionX,
      directionY, testWidth, testHeight, dpr);
  EXPECT_TRUE(result) << "Failed to draw transition";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_transition_move_out_down.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test drawTransition MoveOut Left
TEST_F(RenderTest, DrawTransitionMoveOutLeft) {
  // Create transition images using test_scenes helper
  sk_sp<SkImage> sourceImage;
  sk_sp<SkImage> targetImage;
  bool imagesCreated = createTransitionImages(
      testWidth, testHeight, sourceImage, targetImage);
  ASSERT_TRUE(imagesCreated) << "Failed to create transition images";
  ASSERT_NE(sourceImage, nullptr) << "Source image is null";
  ASSERT_NE(targetImage, nullptr) << "Target image is null";

  // Clear the test canvas
  testCanvas->clear(SK_ColorTRANSPARENT);

  // Draw transition with progress 0.1, direction left (-1, 0)
  float progress = 0.1F;
  common_renderer::TransitionType animationType =
      common_renderer::TransitionType::MOVEOUT;
  float directionX = -1.0F;
  float directionY = 0.0F;
  float dpr = 1.0F; // Test with DPR of 1.0 (no scaling)
  bool result = common_renderer::drawTransition(
      testCanvas, sourceImage, targetImage, progress, animationType, directionX,
      directionY, testWidth, testHeight, dpr);
  EXPECT_TRUE(result) << "Failed to draw transition";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_transition_move_out_left.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Test drawTransition MoveOut Right
TEST_F(RenderTest, DrawTransitionMoveOutRight) {
  // Create transition images using test_scenes helper
  sk_sp<SkImage> sourceImage;
  sk_sp<SkImage> targetImage;
  bool imagesCreated = createTransitionImages(
      testWidth, testHeight, sourceImage, targetImage);
  ASSERT_TRUE(imagesCreated) << "Failed to create transition images";
  ASSERT_NE(sourceImage, nullptr) << "Source image is null";
  ASSERT_NE(targetImage, nullptr) << "Target image is null";

  // Clear the test canvas
  testCanvas->clear(SK_ColorTRANSPARENT);

  // Draw transition with progress 0.1, direction right (1, 0)
  float progress = 0.1F;
  common_renderer::TransitionType animationType =
      common_renderer::TransitionType::MOVEOUT;
  float directionX = 1.0F;
  float directionY = 0.0F;
  float dpr = 1.0F; // Test with DPR of 1.0 (no scaling)
  bool result = common_renderer::drawTransition(
      testCanvas, sourceImage, targetImage, progress, animationType, directionX,
      directionY, testWidth, testHeight, dpr);
  EXPECT_TRUE(result) << "Failed to draw transition";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile = "./fixture/draw_transition_move_out_right.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile)) << "Failed to save output image";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

// Verifies that a partial corner-radius array in JSON (`"r": [...]` with fewer
// than 4 entries) renders identically to the equivalent fully-specified clip.
// Per the schema, a short array fills remaining corners with the last value —
// e.g. [40, 30, 20] expands to [40, 30, 20, 20].
TEST_F(RenderTest, DrawRRectClipFromJsonPartialRadiusArray) {
  std::string json = R"json({
    "type": "Layer", "name": "rrect_partial_radii", "id": 1,
    "horizontalAlignment": { "type": "center", "width": 200 },
    "verticalAlignment": { "type": "center", "height": 150 },
    "rotation": { "cx": 0, "cy": 0, "z": 0, "skewX": 0 },
    "fill": { "type": "solid", "color": "#FF0000" },
    "clip": { "type": "rrect", "enable": true, "r": [40, 30, 20] }
  })json";
  auto scene = skia_parser::parseNode(json, testWidth, testHeight);
  ASSERT_NE(scene, nullptr) << "Failed to parse scene";
  bool result = common_renderer::draw(testCanvas, scene, testWidth, testHeight, false);
  EXPECT_TRUE(result) << "Failed to draw rrect clip with partial radius array";

  // Get pixel data directly from surface
  auto outputPixels = getSurfacePixels();
  EXPECT_FALSE(outputPixels.empty()) << "Failed to get surface pixels";

  // Check if fixture exists
  std::string fixtureFile =
      "./fixture/draw_rrect_clip_from_json_partial_radius_array.png";
  if (!std::filesystem::exists(fixtureFile)) {
    // Fixture doesn't exist, save PNG directly to fixture
    EXPECT_TRUE(saveCanvasToPNG(fixtureFile))
        << "Failed to save rrect clip with partial radius array";
    EXPECT_TRUE(false) << "Fixture file does not exist: " << fixtureFile;
  }

  // Load fixture and compare
  auto fixturePixels = loadPNGPixels(fixtureFile);
  EXPECT_FALSE(fixturePixels.empty()) << "Failed to load fixture image";

  if (!fixturePixels.empty()) {
    EXPECT_TRUE(
        compareImages(outputPixels, fixturePixels, testWidth, testHeight, 0.0F, fixtureFile))
        << "Rendered image does not match expected fixture";
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
