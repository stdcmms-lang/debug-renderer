#include "common_renderer.h"
#include "render_utils.h"
#include <gtest/gtest.h>
#include <memory>
#include <variant>
#include <vector>

namespace {

class RenderUtilsTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Set up test dimensions
    parentWidth = 400.0F;
    parentHeight = 300.0F;
  }

  void TearDown() override {}

  // Helper function to create a circle node with specific positioning
  std::shared_ptr<common_renderer::LayerNode> createCircleNode(
      common_renderer::HorizontalAlign alignH =
          common_renderer::HorizontalAlign::LEFT,
      common_renderer::VerticalAlign alignV =
          common_renderer::VerticalAlign::TOP,
      std::optional<common_renderer::PositionValue> left = std::nullopt,
      std::optional<common_renderer::PositionValue> right = std::nullopt,
      std::optional<common_renderer::PositionValue> top = std::nullopt,
      std::optional<common_renderer::PositionValue> bottom = std::nullopt,
      std::optional<common_renderer::PositionValue> width = std::nullopt,
      std::optional<common_renderer::PositionValue> height = std::nullopt) {
    auto circle = std::make_shared<common_renderer::LayerNode>();
    circle->name = "test_circle";
    circle->id = 1;
    circle->alignH = alignH;
    circle->alignV = alignV;
    circle->left = left;
    circle->right = right;
    circle->top = top;
    circle->bottom = bottom;
    circle->width = width;
    circle->height = height;
    // Set clip to oval for circle
    circle->clip = common_renderer::OvalClip{true};
    return circle;
  }

  // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
  float parentWidth{};
  // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
  float parentHeight{};
};

// Test null pointer handling
TEST_F(RenderUtilsTest, ToTLWHPriorRotationNullPointer) {
  std::vector<float> result =
      common_renderer::toTlwhPriorRotation(nullptr, parentWidth, parentHeight);
  EXPECT_EQ(result.size(), 4);
  EXPECT_FLOAT_EQ(result[0], 0.0F);         // x
  EXPECT_FLOAT_EQ(result[1], 0.0F);         // y
  EXPECT_FLOAT_EQ(result[2], parentWidth);  // width
  EXPECT_FLOAT_EQ(result[3], parentHeight); // height
}

// Test left/top positioning with pixel values
TEST_F(RenderUtilsTest, ToTLWHPriorRotationLeftTopPixelValues) {
  auto circle = createCircleNode(
      common_renderer::HorizontalAlign::LEFT,
      common_renderer::VerticalAlign::TOP,
      std::make_tuple(50.0F, false),                // left: 50 pixels
      std::nullopt, std::make_tuple(30.0F, false),  // top: 30 pixels
      std::nullopt, std::make_tuple(100.0F, false), // width: 100 pixels
      std::make_tuple(80.0F, false)                 // height: 80 pixels
  );

  std::vector<float> result =
      common_renderer::toTlwhPriorRotation(circle, parentWidth, parentHeight);
  EXPECT_EQ(result.size(), 4);
  EXPECT_FLOAT_EQ(result[0], 50.0F);  // x
  EXPECT_FLOAT_EQ(result[1], 30.0F);  // y
  EXPECT_FLOAT_EQ(result[2], 100.0F); // width
  EXPECT_FLOAT_EQ(result[3], 80.0F);  // height
}

// Test left/top positioning with percentage values
TEST_F(RenderUtilsTest, ToTLWHPriorRotationLeftTopPercentageValues) {
  auto circle =
      createCircleNode(common_renderer::HorizontalAlign::LEFT,
                       common_renderer::VerticalAlign::TOP,
                       std::make_tuple(25.0F, true),               // left: 25%
                       std::nullopt, std::make_tuple(20.0F, true), // top: 20%
                       std::nullopt, std::make_tuple(50.0F, true), // width: 50%
                       std::make_tuple(40.0F, true) // height: 40%
      );

  std::vector<float> result =
      common_renderer::toTlwhPriorRotation(circle, parentWidth, parentHeight);
  EXPECT_EQ(result.size(), 4);
  EXPECT_FLOAT_EQ(result[0], 100.0F); // x: 25% of 400 = 100
  EXPECT_FLOAT_EQ(result[1], 60.0F);  // y: 20% of 300 = 60
  EXPECT_FLOAT_EQ(result[2], 200.0F); // width: 50% of 400 = 200
  EXPECT_FLOAT_EQ(result[3], 120.0F); // height: 40% of 300 = 120
}

// Test center alignment
TEST_F(RenderUtilsTest, ToTLWHPriorRotationCenterAlignment) {
  auto circle =
      createCircleNode(common_renderer::HorizontalAlign::CENTER,
                       common_renderer::VerticalAlign::CENTER, std::nullopt,
                       std::nullopt, std::nullopt, std::nullopt,
                       std::make_tuple(120.0F, false), // width: 120 pixels
                       std::make_tuple(90.0F, false)   // height: 90 pixels
      );

  std::vector<float> result =
      common_renderer::toTlwhPriorRotation(circle, parentWidth, parentHeight);
  EXPECT_EQ(result.size(), 4);
  EXPECT_FLOAT_EQ(result[0], 140.0F); // x: (400 - 120) / 2 = 140
  EXPECT_FLOAT_EQ(result[1], 105.0F); // y: (300 - 90) / 2 = 105
  EXPECT_FLOAT_EQ(result[2], 120.0F); // width
  EXPECT_FLOAT_EQ(result[3], 90.0F);  // height
}

// Test right/bottom alignment
TEST_F(RenderUtilsTest, ToTLWHPriorRotationRightBottomAlignment) {
  auto circle = createCircleNode(
      common_renderer::HorizontalAlign::RIGHT,
      common_renderer::VerticalAlign::BOTTOM, std::nullopt,
      std::make_tuple(40.0F, false),               // right: 40 pixels
      std::nullopt, std::make_tuple(20.0F, false), // bottom: 20 pixels
      std::make_tuple(100.0F, false),              // width: 100 pixels
      std::make_tuple(80.0F, false)                // height: 80 pixels
  );

  std::vector<float> result =
      common_renderer::toTlwhPriorRotation(circle, parentWidth, parentHeight);
  EXPECT_EQ(result.size(), 4);
  EXPECT_FLOAT_EQ(result[0], 260.0F); // x: 400 - 40 - 100 = 260
  EXPECT_FLOAT_EQ(result[1], 200.0F); // y: 300 - 20 - 80 = 200
  EXPECT_FLOAT_EQ(result[2], 100.0F); // width
  EXPECT_FLOAT_EQ(result[3], 80.0F);  // height
}

// Test edge alignment (stretch to edges)
TEST_F(RenderUtilsTest, ToTLWHPriorRotationEdgeAlignment) {
  auto circle =
      createCircleNode(common_renderer::HorizontalAlign::EDGE,
                       common_renderer::VerticalAlign::EDGE,
                       std::make_tuple(30.0F, false), // left: 30 pixels
                       std::make_tuple(50.0F, false), // right: 50 pixels
                       std::make_tuple(20.0F, false), // top: 20 pixels
                       std::make_tuple(40.0F, false)  // bottom: 40 pixels
      );

  std::vector<float> result =
      common_renderer::toTlwhPriorRotation(circle, parentWidth, parentHeight);
  EXPECT_EQ(result.size(), 4);
  EXPECT_FLOAT_EQ(result[0], 30.0F);  // x: left value
  EXPECT_FLOAT_EQ(result[1], 20.0F);  // y: top value
  EXPECT_FLOAT_EQ(result[2], 320.0F); // width: 400 - 30 - 50 = 320
  EXPECT_FLOAT_EQ(result[3], 240.0F); // height: 300 - 20 - 40 = 240
}

// Test edge alignment with percentage values
TEST_F(RenderUtilsTest, ToTLWHPriorRotationEdgeAlignmentPercentage) {
  auto circle = createCircleNode(common_renderer::HorizontalAlign::EDGE,
                                 common_renderer::VerticalAlign::EDGE,
                                 std::make_tuple(10.0F, true), // left: 10%
                                 std::make_tuple(15.0F, true), // right: 15%
                                 std::make_tuple(5.0F, true),  // top: 5%
                                 std::make_tuple(10.0F, true)  // bottom: 10%
  );

  std::vector<float> result =
      common_renderer::toTlwhPriorRotation(circle, parentWidth, parentHeight);
  EXPECT_EQ(result.size(), 4);
  EXPECT_FLOAT_EQ(result[0], 40.0F);  // x: 10% of 400 = 40
  EXPECT_FLOAT_EQ(result[1], 15.0F);  // y: 5% of 300 = 15
  EXPECT_FLOAT_EQ(result[2], 300.0F); // width: 400 - 40 - 60 = 300
  EXPECT_FLOAT_EQ(result[3], 255.0F); // height: 300 - 15 - 30 = 255
}

// Test center alignment with percentage values
TEST_F(RenderUtilsTest, ToTLWHPriorRotationCenterPercentage) {
  auto circle = createCircleNode(
      common_renderer::HorizontalAlign::CENTER,
      common_renderer::VerticalAlign::CENTER, std::nullopt, std::nullopt,
      std::nullopt, std::nullopt, std::make_tuple(30.0F, true), // width: 30%
      std::make_tuple(25.0F, true)                              // height: 25%
  );

  std::vector<float> result =
      common_renderer::toTlwhPriorRotation(circle, parentWidth, parentHeight);
  EXPECT_EQ(result.size(), 4);
  EXPECT_FLOAT_EQ(result[0], 140.0F); // x: (400 - 120) / 2 = 140
  EXPECT_FLOAT_EQ(result[1], 112.5F); // y: (300 - 75) / 2 = 112.5
  EXPECT_FLOAT_EQ(result[2], 120.0F); // width: 30% of 400 = 120
  EXPECT_FLOAT_EQ(result[3], 75.0F);  // height: 25% of 300 = 75
}

// Test right/bottom alignment with percentage values
TEST_F(RenderUtilsTest, ToTLWHPriorRotationRightBottomPercentage) {
  auto circle = createCircleNode(
      common_renderer::HorizontalAlign::RIGHT,
      common_renderer::VerticalAlign::BOTTOM, std::nullopt,
      std::make_tuple(15.0F, true),               // right: 15%
      std::nullopt, std::make_tuple(10.0F, true), // bottom: 10%
      std::make_tuple(20.0F, true),               // width: 20%
      std::make_tuple(15.0F, true)                // height: 15%
  );

  std::vector<float> result =
      common_renderer::toTlwhPriorRotation(circle, parentWidth, parentHeight);
  EXPECT_EQ(result.size(), 4);
  EXPECT_FLOAT_EQ(
      result[0],
      260.0F); // x: 400 - 60 - 80 = 260 (15% of 400 = 60, 20% of 400 = 80)
  EXPECT_FLOAT_EQ(
      result[1],
      225.0F); // y: 300 - 30 - 45 = 225 (10% of 300 = 30, 15% of 300 = 45)
  EXPECT_FLOAT_EQ(result[2], 80.0F); // width: 20% of 400 = 80
  EXPECT_FLOAT_EQ(result[3], 45.0F); // height: 15% of 300 = 45
}

// Test mixed pixel and percentage values
TEST_F(RenderUtilsTest, ToTLWHPriorRotationMixedValues) {
  auto circle =
      createCircleNode(common_renderer::HorizontalAlign::LEFT,
                       common_renderer::VerticalAlign::TOP,
                       std::make_tuple(50.0F, false), // left: 50 pixels
                       std::nullopt, std::make_tuple(10.0F, true), // top: 10%
                       std::nullopt, std::make_tuple(25.0F, true), // width: 25%
                       std::make_tuple(60.0F, false) // height: 60 pixels
      );

  std::vector<float> result =
      common_renderer::toTlwhPriorRotation(circle, parentWidth, parentHeight);
  EXPECT_EQ(result.size(), 4);
  EXPECT_FLOAT_EQ(result[0], 50.0F);  // x: left value
  EXPECT_FLOAT_EQ(result[1], 30.0F);  // y: 10% of 300 = 30
  EXPECT_FLOAT_EQ(result[2], 100.0F); // width: 25% of 400 = 100
  EXPECT_FLOAT_EQ(result[3], 60.0F);  // height: height value
}

TEST_F(RenderUtilsTest, LoadImageFromDataCache) {
  // Clear cache to start with a clean state
  common_renderer::clearImageCache();

  // Create a simple test image data URL (1x1 red pixel PNG)
  std::string testDataUrl = "data:image/"
                            "png;base64,"
                            "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADU"
                            "lEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg==";

  // First load - should cache the image
  auto image1 = common_renderer::loadImageFromData(testDataUrl);
  EXPECT_TRUE(image1 != nullptr) << "First image load should succeed";
  EXPECT_EQ(image1->width(), 1) << "Image should be 1x1 pixel";
  EXPECT_EQ(image1->height(), 1) << "Image should be 1x1 pixel";

  // Second load - should return cached image
  auto image2 = common_renderer::loadImageFromData(testDataUrl);
  EXPECT_TRUE(image2 != nullptr) << "Second image load should succeed";

  // Both should be the same object (reference equality)
  EXPECT_EQ(image1.get(), image2.get())
      << "Cached images should be the same object (reference equality)";

  // Test with invalid data URL
  auto invalidImage = common_renderer::loadImageFromData("invalid-data-url");
  EXPECT_TRUE(invalidImage == nullptr)
      << "Invalid data URL should return nullptr";

  // Test with empty string
  auto emptyImage = common_renderer::loadImageFromData("");
  EXPECT_TRUE(emptyImage == nullptr) << "Empty string should return nullptr";

  // Test cache persistence - same image should still be cached
  auto image3 = common_renderer::loadImageFromData(testDataUrl);
  EXPECT_TRUE(image3 != nullptr) << "Third image load should succeed";
  EXPECT_EQ(image1.get(), image3.get())
      << "Third load should return same cached object";
}

// Test parse_color function with hex colors
TEST_F(RenderUtilsTest, ParseColorHex) {
  // Test 6-digit hex color
  SkColor color1 = common_renderer::parseColor("#FF0000");
  EXPECT_EQ(color1, SK_ColorRED) << "Should parse red hex color";

  // Test 8-digit hex color with alpha
  SkColor color2 = common_renderer::parseColor("#FF000080");
  EXPECT_EQ(SkColorGetA(color2), 0x80) << "Should parse alpha channel";
  EXPECT_EQ(SkColorGetR(color2), 0xFF) << "Should parse red channel";
  EXPECT_EQ(SkColorGetG(color2), 0x00) << "Should parse green channel";
  EXPECT_EQ(SkColorGetB(color2), 0x00) << "Should parse blue channel";

  // Test empty string
  SkColor color3 = common_renderer::parseColor("");
  EXPECT_EQ(color3, SK_ColorTRANSPARENT)
      << "Empty string should return transparent";

  // Test invalid hex
  SkColor color4 = common_renderer::parseColor("#GGGGGG");
  EXPECT_EQ(color4, SK_ColorTRANSPARENT)
      << "Invalid hex should return transparent";
}

// Test parse_color function with RGB colors
TEST_F(RenderUtilsTest, ParseColorRGB) {
  // Test basic RGB
  SkColor color1 = common_renderer::parseColor("rgb(255, 0, 0)");
  EXPECT_EQ(color1, SK_ColorRED) << "Should parse red RGB color";

  // Test RGB with spaces
  SkColor color2 = common_renderer::parseColor("rgb( 0 , 255 , 0 )");
  EXPECT_EQ(color2, SK_ColorGREEN) << "Should parse green RGB with spaces";

  // Test RGB without spaces
  SkColor color3 = common_renderer::parseColor("rgb(0,0,255)");
  EXPECT_EQ(color3, SK_ColorBLUE) << "Should parse blue RGB without spaces";

  // Test invalid RGB format
  SkColor color4 = common_renderer::parseColor("rgb(255,0)");
  EXPECT_EQ(color4, SK_ColorTRANSPARENT)
      << "Invalid RGB should return transparent";

  // Test RGB with wrong number of values
  SkColor color5 = common_renderer::parseColor("rgb(255,0,0,0)");
  EXPECT_EQ(color5, SK_ColorTRANSPARENT)
      << "RGB with 4 values should return transparent";
}

// Test parse_color function with RGBA colors
TEST_F(RenderUtilsTest, ParseColorRGBA) {
  // Test basic RGBA
  SkColor color1 = common_renderer::parseColor("rgba(255, 0, 0, 1.0)");
  EXPECT_EQ(color1, SK_ColorRED) << "Should parse red RGBA with full alpha";

  // Test RGBA with transparency
  SkColor color2 = common_renderer::parseColor("rgba(0, 255, 0, 0.5)");
  EXPECT_EQ(SkColorGetA(color2), 127) << "Should parse 50% alpha (127/255)";
  EXPECT_EQ(SkColorGetR(color2), 0x00) << "Should parse red channel";
  EXPECT_EQ(SkColorGetG(color2), 0xFF) << "Should parse green channel";
  EXPECT_EQ(SkColorGetB(color2), 0x00) << "Should parse blue channel";

  // Test RGBA with spaces
  SkColor color3 = common_renderer::parseColor("rgba( 0 , 0 , 255 , 0.25 )");
  EXPECT_EQ(SkColorGetA(color3), 63) << "Should parse 25% alpha (63/255)";
  EXPECT_EQ(SkColorGetB(color3), 0xFF) << "Should parse blue channel";

  // Test RGBA without spaces
  SkColor color4 = common_renderer::parseColor("rgba(150,55,20,0.5)");
  EXPECT_EQ(SkColorGetA(color4), 127) << "Should parse 50% alpha";
  EXPECT_EQ(SkColorGetR(color4), 150) << "Should parse red channel";
  EXPECT_EQ(SkColorGetG(color4), 55) << "Should parse green channel";
  EXPECT_EQ(SkColorGetB(color4), 20) << "Should parse blue channel";

  // Test invalid RGBA format
  SkColor color5 = common_renderer::parseColor("rgba(255,0,0)");
  EXPECT_EQ(color5, SK_ColorTRANSPARENT)
      << "RGBA with 3 values should return transparent";

  // Test RGBA with wrong number of values
  SkColor color6 = common_renderer::parseColor("rgba(255,0,0,0.5,0.5)");
  EXPECT_EQ(color6, SK_ColorTRANSPARENT)
      << "RGBA with 5 values should return transparent";
}

// Test create_paint_from_fill with SolidColor
TEST_F(RenderUtilsTest, CreatePaintFromFillSolidColor) {
  common_renderer::SolidColor solidColor;
  solidColor.color = "#FF0000";

  common_renderer::Fill fill = solidColor;
  SkPaint paint =
      common_renderer::createPaintFromFill(fill, 0.0F, 0.0F, 100.0F, 100.0F);

  EXPECT_EQ(paint.getColor(), SK_ColorRED);
  EXPECT_TRUE(paint.getShader() == nullptr);
}

// Test create_paint_from_fill with GradientColor
TEST_F(RenderUtilsTest, CreatePaintFromFillGradientColor) {
  common_renderer::GradientColor gradientColor;
  gradientColor.startColor = "#FF0000";
  gradientColor.endColor = "#0000FF";
  gradientColor.startPoint = {0.0F, 0.0F};
  gradientColor.endPoint = {1.0F, 1.0F};

  common_renderer::Fill fill = gradientColor;
  SkPaint paint =
      common_renderer::createPaintFromFill(fill, 0.0F, 0.0F, 100.0F, 100.0F);

  EXPECT_TRUE(paint.getShader() != nullptr);
}

// Test create_paint_from_fill with ImageFill
TEST_F(RenderUtilsTest, CreatePaintFromFillImageFill) {
  common_renderer::ImageFill imageFill;
  imageFill.image = common_renderer::loadImageFromData(
      "data:image/"
      "png;base64,"
      "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQV"
      "R42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg==");
  imageFill.fit = "cover";

  common_renderer::Fill fill = imageFill;
  SkPaint paint =
      common_renderer::createPaintFromFill(fill, 0.0F, 0.0F, 100.0F, 100.0F);

  // Image fill should create a shader if image loads successfully
  // If image fails to load, it falls back to transparent color
  // We can't easily test the shader without mocking, but we can verify the
  // function doesn't crash
  EXPECT_TRUE(paint.getShader() != nullptr ||
              paint.getColor() == SK_ColorTRANSPARENT);
}

// Test create_paint_from_fill with ImageFill and different fit types
TEST_F(RenderUtilsTest, CreatePaintFromFillImageFillContain) {
  common_renderer::ImageFill imageFill;
  imageFill.image = common_renderer::loadImageFromData(
      "data:image/"
      "png;base64,"
      "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQV"
      "R42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg==");
  imageFill.fit = "contain";

  common_renderer::Fill fill = imageFill;
  SkPaint paint =
      common_renderer::createPaintFromFill(fill, 0.0F, 0.0F, 100.0F, 100.0F);

  EXPECT_TRUE(paint.getShader() != nullptr ||
              paint.getColor() == SK_ColorTRANSPARENT);
}

// Test backdrop effects with clip disabled
TEST_F(RenderUtilsTest, BackdropEffectsWithClipDisabled) {
  common_renderer::RRectClip clip{};
  clip.enable = false;
  common_renderer::Clip clipVariant = clip;

  // All effects should be disabled for backdrop when clip is disabled
  common_renderer::Blur blur{4.0F};
  common_renderer::DisplacementMap displacementMap{10.0F, 1.0F, 1.0F, 1};
  common_renderer::CRTEffect crt{0.5F, 0.5F, 0.5F, 2.0F};
  common_renderer::LiquidGlass liquidGlass{1.5F, 0.0F, 10.0F};

  EXPECT_TRUE(common_renderer::isEffectDisabled(common_renderer::Effect(blur),
                                                clipVariant, true));
  EXPECT_TRUE(common_renderer::isEffectDisabled(
      common_renderer::Effect(displacementMap), clipVariant, true));
  EXPECT_TRUE(common_renderer::isEffectDisabled(common_renderer::Effect(crt),
                                                clipVariant, true));
  EXPECT_TRUE(common_renderer::isEffectDisabled(
      common_renderer::Effect(liquidGlass), clipVariant, true));
}

// Test backdrop effects with clip enabled (non-path)
TEST_F(RenderUtilsTest, BackdropEffectsWithClipEnabled) {
  common_renderer::RRectClip clip{};
  clip.enable = true;
  common_renderer::Clip clipVariant = clip;

  common_renderer::Blur blur{4.0F};
  common_renderer::DisplacementMap displacementMap{10.0F, 1.0F, 1.0F, 1};
  common_renderer::CRTEffect crt{0.5F, 0.5F, 0.5F, 2.0F};
  common_renderer::LiquidGlass liquidGlass{1.5F, 0.0F, 10.0F};

  // All effects should be enabled for backdrop when clip is enabled (non-path)
  EXPECT_FALSE(common_renderer::isEffectDisabled(common_renderer::Effect(blur),
                                                 clipVariant, true));
  EXPECT_FALSE(common_renderer::isEffectDisabled(
      common_renderer::Effect(displacementMap), clipVariant, true));
  EXPECT_FALSE(common_renderer::isEffectDisabled(common_renderer::Effect(crt),
                                                 clipVariant, true));
  EXPECT_FALSE(common_renderer::isEffectDisabled(
      common_renderer::Effect(liquidGlass), clipVariant, true));
}

// Test backdrop effects with path clip
TEST_F(RenderUtilsTest, BackdropEffectsWithPathClip) {
  common_renderer::PathClip clip{};
  clip.enable = true;
  common_renderer::Clip clipVariant = clip;

  common_renderer::Blur blur{4.0F};
  common_renderer::DisplacementMap displacementMap{10.0F, 1.0F, 1.0F, 1};
  common_renderer::CRTEffect crt{0.5F, 0.5F, 0.5F, 2.0F};
  common_renderer::LiquidGlass liquidGlass{1.5F, 0.0F, 10.0F};

  // Non-LiquidGlass effects should be enabled
  EXPECT_FALSE(common_renderer::isEffectDisabled(common_renderer::Effect(blur),
                                                 clipVariant, true));
  EXPECT_FALSE(common_renderer::isEffectDisabled(
      common_renderer::Effect(displacementMap), clipVariant, true));
  EXPECT_FALSE(common_renderer::isEffectDisabled(common_renderer::Effect(crt),
                                                 clipVariant, true));

  // LiquidGlass should be disabled when clip is a path
  EXPECT_TRUE(common_renderer::isEffectDisabled(
      common_renderer::Effect(liquidGlass), clipVariant, true));
}

// Test foreground effects with clip disabled
TEST_F(RenderUtilsTest, ForegroundEffectsWithClipDisabled) {
  common_renderer::RRectClip clip{};
  clip.enable = false;
  common_renderer::Clip clipVariant = clip;

  common_renderer::Blur blur{4.0F};
  common_renderer::DisplacementMap displacementMap{10.0F, 1.0F, 1.0F, 1};
  common_renderer::CRTEffect crt{0.5F, 0.5F, 0.5F, 2.0F};
  common_renderer::LiquidGlass liquidGlass{1.5F, 0.0F, 10.0F};

  // Non-LiquidGlass effects should be enabled
  EXPECT_FALSE(common_renderer::isEffectDisabled(common_renderer::Effect(blur),
                                                 clipVariant, false));
  EXPECT_FALSE(common_renderer::isEffectDisabled(
      common_renderer::Effect(displacementMap), clipVariant, false));
  EXPECT_FALSE(common_renderer::isEffectDisabled(common_renderer::Effect(crt),
                                                 clipVariant, false));

  // LiquidGlass should be disabled when clip is disabled
  EXPECT_TRUE(common_renderer::isEffectDisabled(
      common_renderer::Effect(liquidGlass), clipVariant, false));
}

// Test foreground effects with clip enabled
TEST_F(RenderUtilsTest, ForegroundEffectsWithClipEnabled) {
  common_renderer::RRectClip clip{};
  clip.enable = true;
  common_renderer::Clip clipVariant = clip;

  common_renderer::Blur blur{4.0F};
  common_renderer::DisplacementMap displacementMap{10.0F, 1.0F, 1.0F, 1};
  common_renderer::CRTEffect crt{0.5F, 0.5F, 0.5F, 2.0F};
  common_renderer::LiquidGlass liquidGlass{1.5F, 0.0F, 10.0F};

  // All effects should be enabled for foreground when clip is enabled
  EXPECT_FALSE(common_renderer::isEffectDisabled(common_renderer::Effect(blur),
                                                 clipVariant, false));
  EXPECT_FALSE(common_renderer::isEffectDisabled(
      common_renderer::Effect(displacementMap), clipVariant, false));
  EXPECT_FALSE(common_renderer::isEffectDisabled(common_renderer::Effect(crt),
                                                 clipVariant, false));
  EXPECT_FALSE(common_renderer::isEffectDisabled(
      common_renderer::Effect(liquidGlass), clipVariant, false));
}

// Test foreground effects with path clip
TEST_F(RenderUtilsTest, ForegroundEffectsWithPathClip) {
  common_renderer::PathClip clip{};
  clip.enable = true;
  common_renderer::Clip clipVariant = clip;

  common_renderer::Blur blur{4.0F};
  common_renderer::DisplacementMap displacementMap{10.0F, 1.0F, 1.0F, 1};
  common_renderer::CRTEffect crt{0.5F, 0.5F, 0.5F, 2.0F};
  common_renderer::LiquidGlass liquidGlass{1.5F, 0.0F, 10.0F};

  // All effects should be enabled for foreground when clip is enabled (even if
  // path)
  EXPECT_FALSE(common_renderer::isEffectDisabled(common_renderer::Effect(blur),
                                                 clipVariant, false));
  EXPECT_FALSE(common_renderer::isEffectDisabled(
      common_renderer::Effect(displacementMap), clipVariant, false));
  EXPECT_FALSE(common_renderer::isEffectDisabled(common_renderer::Effect(crt),
                                                 clipVariant, false));
  EXPECT_FALSE(common_renderer::isEffectDisabled(
      common_renderer::Effect(liquidGlass), clipVariant, false));
}

// Test filterEffects with backdrop effects
TEST_F(RenderUtilsTest, FilterBackdropEffects) {
  common_renderer::RRectClip clip{};
  clip.enable = false; // Clip disabled
  common_renderer::Clip clipVariant = clip;

  std::vector<common_renderer::Effect> effects = {
      common_renderer::Blur{4.0F},
      common_renderer::DisplacementMap{10.0F, 1.0F, 1.0F, 1},
      common_renderer::CRTEffect{0.5F, 0.5F, 0.5F, 2.0F},
      common_renderer::LiquidGlass{1.5F, 0.0F, 10.0F}};

  // All effects should be filtered out when clip is disabled for backdrop
  auto filtered = common_renderer::filterEffects(effects, clipVariant, true);
  EXPECT_EQ(filtered.size(), 0);
}

// Test filterEffects with backdrop effects and enabled clip
TEST_F(RenderUtilsTest, FilterBackdropEffectsWithEnabledClip) {
  common_renderer::RRectClip clip{};
  clip.enable = true;
  common_renderer::Clip clipVariant = clip;

  std::vector<common_renderer::Effect> effects = {
      common_renderer::Blur{4.0F},
      common_renderer::DisplacementMap{10.0F, 1.0F, 1.0F, 1},
      common_renderer::CRTEffect{0.5F, 0.5F, 0.5F, 2.0F},
      common_renderer::LiquidGlass{1.5F, 0.0F, 10.0F}};

  // All effects should pass through when clip is enabled (non-path) for
  // backdrop
  auto filtered = common_renderer::filterEffects(effects, clipVariant, true);
  EXPECT_EQ(filtered.size(), 4);
}

// Test filterEffects with backdrop effects and path clip
TEST_F(RenderUtilsTest, FilterBackdropEffectsWithPathClip) {
  common_renderer::PathClip clip{};
  clip.enable = true;
  common_renderer::Clip clipVariant = clip;

  std::vector<common_renderer::Effect> effects = {
      common_renderer::Blur{4.0F},
      common_renderer::DisplacementMap{10.0F, 1.0F, 1.0F, 1},
      common_renderer::CRTEffect{0.5F, 0.5F, 0.5F, 2.0F},
      common_renderer::LiquidGlass{1.5F, 0.0F, 10.0F}};

  // LiquidGlass should be filtered out, others should pass
  auto filtered = common_renderer::filterEffects(effects, clipVariant, true);
  EXPECT_EQ(filtered.size(), 3);

  // Verify LiquidGlass is not in the filtered list
  bool hasLiquidGlass = false;
  for (const auto &effect : filtered) {
    if (std::holds_alternative<common_renderer::LiquidGlass>(effect)) {
      hasLiquidGlass = true;
      break;
    }
  }
  EXPECT_FALSE(hasLiquidGlass);
}

// Test filterEffects with foreground effects and disabled clip
TEST_F(RenderUtilsTest, FilterForegroundEffectsWithDisabledClip) {
  common_renderer::RRectClip clip{};
  clip.enable = false;
  common_renderer::Clip clipVariant = clip;

  std::vector<common_renderer::Effect> effects = {
      common_renderer::Blur{4.0F},
      common_renderer::DisplacementMap{10.0F, 1.0F, 1.0F, 1},
      common_renderer::CRTEffect{0.5F, 0.5F, 0.5F, 2.0F},
      common_renderer::LiquidGlass{1.5F, 0.0F, 10.0F}};

  // Only LiquidGlass should be filtered out
  auto filtered = common_renderer::filterEffects(effects, clipVariant, false);
  EXPECT_EQ(filtered.size(), 3);

  // Verify LiquidGlass is not in the filtered list
  bool hasLiquidGlass = false;
  for (const auto &effect : filtered) {
    if (std::holds_alternative<common_renderer::LiquidGlass>(effect)) {
      hasLiquidGlass = true;
      break;
    }
  }
  EXPECT_FALSE(hasLiquidGlass);
}

// Test filterEffects with foreground effects and enabled clip
TEST_F(RenderUtilsTest, FilterForegroundEffectsWithEnabledClip) {
  common_renderer::RRectClip clip{};
  clip.enable = true;
  common_renderer::Clip clipVariant = clip;

  std::vector<common_renderer::Effect> effects = {
      common_renderer::Blur{4.0F},
      common_renderer::DisplacementMap{10.0F, 1.0F, 1.0F, 1},
      common_renderer::CRTEffect{0.5F, 0.5F, 0.5F, 2.0F},
      common_renderer::LiquidGlass{1.5F, 0.0F, 10.0F}};

  // All effects should pass through when clip is enabled for foreground
  auto filtered = common_renderer::filterEffects(effects, clipVariant, false);
  EXPECT_EQ(filtered.size(), 4);
}

// Test filterEffects with empty effects vector
TEST_F(RenderUtilsTest, FilterEmptyEffects) {
  common_renderer::RRectClip clip{};
  clip.enable = true;
  common_renderer::Clip clipVariant = clip;

  std::vector<common_renderer::Effect> effects;

  auto filtered = common_renderer::filterEffects(effects, clipVariant, true);
  EXPECT_EQ(filtered.size(), 0);
}

// Test filterEffects with oval clip
TEST_F(RenderUtilsTest, FilterEffectsWithOvalClip) {
  common_renderer::OvalClip clip{};
  clip.enable = true;
  common_renderer::Clip clipVariant = clip;

  std::vector<common_renderer::Effect> effects = {
      common_renderer::Blur{4.0F},
      common_renderer::LiquidGlass{1.5F, 0.0F, 10.0F}};

  // For backdrop: all should pass (oval is not a path)
  auto filteredBackdrop =
      common_renderer::filterEffects(effects, clipVariant, true);
  EXPECT_EQ(filteredBackdrop.size(), 2);

  // For foreground: all should pass
  auto filteredForeground =
      common_renderer::filterEffects(effects, clipVariant, false);
  EXPECT_EQ(filteredForeground.size(), 2);
}

} // namespace
