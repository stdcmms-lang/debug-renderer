#pragma once

#include "common_renderer.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#if defined(__APPLE__)
#include "include/ports/SkFontMgr_mac_ct.h"
#else
#include "include/ports/SkFontMgr_fontconfig.h"
#include "include/ports/SkFontScanner_FreeType.h"
#endif
#include "src/base/SkBase64.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>
#include <variant>
#include <vector>

inline sk_sp<SkFontMgr> MakeTestFontMgr() {
#if defined(__APPLE__)
  return SkFontMgr_New_CoreText(nullptr);
#else
  return SkFontMgr_New_FontConfig(nullptr, SkFontScanner_Make_FreeType());
#endif
}

class RenderTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Set up a test canvas with fixed dimensions
    testWidth = 400;
    testHeight = 300;

    // Create a software surface for testing
    testSurface =
        SkSurfaces::Raster(SkImageInfo::MakeN32Premul(static_cast<int>(testWidth), static_cast<int>(testHeight)));
    testCanvas = testSurface->getCanvas();

    // Clear canvas with white background
    testCanvas->clear(SK_ColorWHITE);
  }

  void TearDown() override {
    testCanvas = nullptr;
    testSurface = nullptr;
  }

  // Helper function to create a simple red circle node
  std::shared_ptr<common_renderer::LayerNode> createRedCircle() {
    auto circle = std::make_shared<common_renderer::LayerNode>();
    circle->name = "test_circle";
    circle->id = 1;

    // Set positioning - center the circle
    circle->alignH = common_renderer::HorizontalAlign::CENTER;
    circle->alignV = common_renderer::VerticalAlign::CENTER;
    circle->width = {100.0F, false};  // 100 pixels wide
    circle->height = {200.0F, false}; // 200 pixels tall

    // Set rotation (no rotation for this test)
    circle->rotation.cx = 0.0F;
    circle->rotation.cy = 0.0F;
    circle->rotation.z = 0.0F;
    circle->rotation.skewX = 0.0F;

    // Set color to red
    common_renderer::SolidColor color;
    color.color = "#FF0000"; // red
    circle->fill = common_renderer::Fill(color);

    // Set clip to oval for circle
    circle->clip = common_renderer::OvalClip{true};

    // No stroke (filled circle)
    circle->stroke = std::nullopt;

    return circle;
  }

  // Helper function to create a basic red rounded rectangle
  std::shared_ptr<common_renderer::LayerNode> createRedRoundedRect() {
    auto rect = std::make_shared<common_renderer::LayerNode>();
    rect->name = "test_rounded_rect";
    rect->id = 3;

    // Set positioning - center the rectangle
    rect->alignH = common_renderer::HorizontalAlign::CENTER;
    rect->alignV = common_renderer::VerticalAlign::CENTER;
    rect->width = {150.0F, false};  // 150 pixels wide
    rect->height = {100.0F, false}; // 100 pixels tall

    // Set rotation (no rotation for this test)
    rect->rotation.cx = 0.0F;
    rect->rotation.cy = 0.0F;
    rect->rotation.z = 0.0F;
    rect->rotation.skewX = 0.0F;

    // Set color to red
    common_renderer::SolidColor color;
    color.color = "#FF0000"; // red
    rect->fill = common_renderer::Fill(color);

    // Set clip to rrect for rounded rectangle
    rect->clip = common_renderer::RRectClip{{20.0F, 20.0F, 20.0F, 20.0F}, true};

    // No stroke (filled rectangle)
    rect->stroke = std::nullopt;

    return rect;
  }

  // Helper function to create a group with children for snapshot testing
  std::shared_ptr<common_renderer::LayerNode> createGroupWithChildren() {
    auto group = std::make_shared<common_renderer::LayerNode>();
    group->name = "test_group";
    group->id = 100;

    // Set positioning
    group->alignH = common_renderer::HorizontalAlign::LEFT;
    group->alignV = common_renderer::VerticalAlign::TOP;
    group->left = {50.0F, false};
    group->top = {50.0F, false};
    group->width = {200.0F, false};
    group->height = {150.0F, false};

    group->rotation.cx = 0.0F;
    group->rotation.cy = 0.0F;
    group->rotation.z = 0.0F;
    group->rotation.skewX = 0.0F;

    // Create first child - red circle
    auto circle = createRedCircle();
    circle->id = 101;
    circle->alignH = common_renderer::HorizontalAlign::LEFT;
    circle->alignV = common_renderer::VerticalAlign::TOP;
    circle->left = {20.0F, false};
    circle->top = {20.0F, false};
    circle->width = {50.0F, false};
    circle->height = {50.0F, false};

    // Create second child - blue rounded rectangle
    auto rect = std::make_shared<common_renderer::LayerNode>();
    rect->name = "rect_child";
    rect->id = 102;
    rect->alignH = common_renderer::HorizontalAlign::LEFT;
    rect->alignV = common_renderer::VerticalAlign::TOP;
    rect->left = {100.0F, false};
    rect->top = {50.0F, false};
    rect->width = {80.0F, false};
    rect->height = {60.0F, false};

    rect->rotation.cx = 0.0F;
    rect->rotation.cy = 0.0F;
    rect->rotation.z = 0.0F;
    rect->rotation.skewX = 0.0F;

    common_renderer::SolidColor rectColor;
    rectColor.color = "#0000FF"; // blue
    rect->fill = common_renderer::Fill(rectColor);

    rect->clip = common_renderer::RRectClip{{10.0F, 10.0F, 10.0F, 10.0F}, true};
    rect->stroke = std::nullopt;

    group->children.push_back(circle);
    group->children.push_back(rect);

    return group;
  }

  // Helper function to get pixel data directly from surface
  std::vector<uint32_t> getSurfacePixels() {
    std::vector<uint32_t> pixels;
    if (!testSurface) {
      return pixels;
    }

    // Create an image from the surface
    sk_sp<SkImage> image = testSurface->makeImageSnapshot();
    if (!image) {
      return pixels;
    }

    // Get pixel data
    SkPixmap pixmap;
    if (!image->peekPixels(&pixmap)) {
      // If peekPixels fails, use readPixels as fallback
      SkImageInfo info = image->imageInfo();
      std::vector<uint32_t> tempPixels(info.width() * info.height());

      if (!image->readPixels(info, tempPixels.data(), info.minRowBytes(), 0,
                             0)) {
        std::cerr << "Failed to read pixels from surface" << std::endl;
        return pixels;
      }

      pixels = std::move(tempPixels);
    } else {
      // Copy pixels to vector
      const auto *pixelPtr = static_cast<const uint32_t *>(pixmap.addr());
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      pixels.assign(pixelPtr, pixelPtr + (pixmap.width() * pixmap.height()));
    }

    return pixels;
  }

  // Helper function to save canvas to PNG (only used when fixture is missing)
  bool saveCanvasToPNG(const std::string &filename) {
    if (!testSurface) {
      return false;
    }

    // Create an image from the surface
    sk_sp<SkImage> image = testSurface->makeImageSnapshot();
    if (!image) {
      return false;
    }

    // Encode to PNG
    sk_sp<SkData> data = SkPngEncoder::Encode(nullptr, image.get(), {});
    if (!data) {
      return false;
    }

    // Write to file
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
      std::cerr << "Failed to open file for writing: " << filename << std::endl;
      return false;
    }

    file.write(static_cast<const char *>(data->data()),
               static_cast<std::streamsize>(data->size()));
    file.close();

    return true;
  }

  // Helper function to load PNG and get pixel data
  std::vector<uint32_t> loadPNGPixels(const std::string &filename) {
    std::vector<uint32_t> pixels;

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
      std::cerr << "Failed to open file: " << filename << std::endl;
      return pixels;
    }

    // Read file into buffer
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), static_cast<std::streamsize>(size));
    file.close();

    // Decode PNG
    sk_sp<SkData> data = SkData::MakeWithoutCopy(buffer.data(), size);
    sk_sp<SkImage> image = SkImages::DeferredFromEncodedData(data);
    if (!image) {
      std::cerr << "Failed to decode PNG: " << filename << std::endl;
      return pixels;
    }

    // Get pixel data
    SkPixmap pixmap;
    if (!image->peekPixels(&pixmap)) {
      // peekPixels usually fails, so use readPixels as fallback
      SkImageInfo info = image->imageInfo();
      std::vector<uint32_t> tempPixels(info.width() * info.height());

      if (!image->readPixels(info, tempPixels.data(), info.minRowBytes(), 0,
                             0)) {
        std::cerr << "Failed to read pixels from image: " << filename
                  << std::endl;
        return pixels;
      }

      pixels = std::move(tempPixels);
    } else {
      // Copy pixels to vector
      const auto *pixelPtr = static_cast<const uint32_t *>(pixmap.addr());
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      pixels.assign(pixelPtr, pixelPtr + (pixmap.width() * pixmap.height()));
    }

    return pixels;
  }

  // Helper function to derive the _last_run output path from a fixture path
  static std::string deriveLastRunPath(const std::string &fixturePath) {
    const std::string ext = ".png";
    if (fixturePath.size() >= ext.size() &&
        fixturePath.compare(fixturePath.size() - ext.size(), ext.size(), ext) ==
            0) {
      return fixturePath.substr(0, fixturePath.size() - ext.size()) +
             "_last_run.png";
    }
    return fixturePath + "_last_run.png";
  }

  // Helper function to compare two images pixel by pixel. When the comparison
  // fails and `fixturePath` is non-empty, the current surface is also written
  // to `<fixturePath without .png>_last_run.png` to aid debugging.
  bool compareImages(const std::vector<uint32_t> &pixels1,
                     const std::vector<uint32_t> &pixels2, int width,
                     int height, float tolerance = 0.0F,
                     const std::string &fixturePath = "") {
    bool matched = true;

    if (pixels1.size() != pixels2.size()) {
      std::cerr << "Image size mismatch: " << pixels1.size() << " vs "
                << pixels2.size() << std::endl;
      matched = false;
    } else {
      int differences = 0;
      int maxDifferences =
          static_cast<int>(static_cast<float>(width * height) * tolerance);

      for (size_t i = 0; i < pixels1.size(); ++i) {
        if (pixels1[i] != pixels2[i]) {
          differences++;
          if (tolerance == 0.0F) {
            std::cerr << "Pixel mismatch at index " << i << ": 0x" << std::hex
                      << pixels1[i] << " vs 0x" << pixels2[i] << std::dec
                      << std::endl;
            matched = false;
            break;
          }
        }
      }

      if (matched && tolerance > 0.0F && differences > maxDifferences) {
        std::cerr << "Too many pixel differences: " << differences << " > "
                  << maxDifferences << std::endl;
        matched = false;
      }
    }

    if (!matched && !fixturePath.empty()) {
      std::string lastRunPath = deriveLastRunPath(fixturePath);
      if (saveCanvasToPNG(lastRunPath)) {
        std::cerr << "Wrote rendered output to: " << lastRunPath << std::endl;
      } else {
        std::cerr << "Failed to write last-run image to: " << lastRunPath
                  << std::endl;
      }
    }

    return matched;
  }

  // Helper function to load image file and convert to base64 data URL
  std::string loadImageAsDataUrl(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
      std::cerr << "Failed to open image file: " << filename << std::endl;
      return "";
    }

    // Read file into buffer
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), static_cast<std::streamsize>(size));
    file.close();

    // Convert to base64 using a simple implementation
    size_t encodedLength = SkBase64::EncodedSize(size);
    std::string base64(encodedLength, '\0');
    SkBase64::Encode(buffer.data(), size, base64.data());

    // Determine MIME type based on file extension
    std::string mimeType = "image/jpeg"; // default
    if (filename.length() >= 4 &&
        filename.substr(filename.length() - 4) == ".png") {
      mimeType = "image/png";
    } else if (filename.length() >= 4 &&
               filename.substr(filename.length() - 4) == ".gif") {
      mimeType = "image/gif";
    } else if (filename.length() >= 5 &&
               filename.substr(filename.length() - 5) == ".webp") {
      mimeType = "image/webp";
    }

    // Create data URL
    return "data:" + mimeType + ";base64," + base64;
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

  // Helper function to create a circle with gradient
  std::shared_ptr<common_renderer::LayerNode> createGradientCircle() {
    auto circle = std::make_shared<common_renderer::LayerNode>();
    circle->name = "gradient_circle";
    circle->id = 100;

    // Set positioning - center the circle
    circle->alignH = common_renderer::HorizontalAlign::CENTER;
    circle->alignV = common_renderer::VerticalAlign::CENTER;
    circle->width = {120.0F, false};  // 120 pixels wide
    circle->height = {120.0F, false}; // 120 pixels tall

    // Set rotation (no rotation for this test)
    circle->rotation.cx = 0.0F;
    circle->rotation.cy = 0.0F;
    circle->rotation.z = 0.0F;
    circle->rotation.skewX = 0.0F;

    // Set gradient color
    common_renderer::GradientColor gradientColor;
    gradientColor.startColor = "#FF0000";    // red
    gradientColor.endColor = "#0000FF";      // blue
    gradientColor.startPoint = {0.0F, 0.0F}; // top-left
    gradientColor.endPoint = {1.0F, 1.0F};   // bottom-right
    circle->fill = common_renderer::Fill(gradientColor);

    // Set clip to oval for circle
    circle->clip = common_renderer::OvalClip{true};

    // No stroke (filled circle)
    circle->stroke = std::nullopt;

    return circle;
  }

  // Helper function to create a rounded rectangle with gradient
  std::shared_ptr<common_renderer::LayerNode> createGradientRoundedRect() {
    auto rect = std::make_shared<common_renderer::LayerNode>();
    rect->name = "gradient_rect";
    rect->id = 101;

    // Set positioning - center the rectangle
    rect->alignH = common_renderer::HorizontalAlign::CENTER;
    rect->alignV = common_renderer::VerticalAlign::CENTER;
    rect->width = {150.0F, false};  // 150 pixels wide
    rect->height = {100.0F, false}; // 100 pixels tall

    // Set rotation (no rotation for this test)
    rect->rotation.cx = 0.0F;
    rect->rotation.cy = 0.0F;
    rect->rotation.z = 0.0F;
    rect->rotation.skewX = 0.0F;

    // Set gradient color
    common_renderer::GradientColor gradientColor;
    gradientColor.startColor = "#00FF00";    // green
    gradientColor.endColor = "#FF0000";      // red
    gradientColor.startPoint = {0.0F, 0.0F}; // top-left
    gradientColor.endPoint = {1.0F, 1.0F};   // bottom-right
    rect->fill = common_renderer::Fill(gradientColor);

    // Set trim (full rectangle)
    // Set corner radii - all corners rounded
    rect->clip = common_renderer::RRectClip{{20.0F, 20.0F, 20.0F, 20.0F}, true};

    // No stroke (filled rectangle)
    rect->stroke = std::nullopt;

    return rect;
  }

  // Helper function to draw a state test circle in the top right corner
  // This is used to test canvas state restoration after drawing operations
  bool drawStateTestCircle() {
    auto stateTestCircle = std::make_shared<common_renderer::LayerNode>();
    stateTestCircle->name = "state_test_circle";
    stateTestCircle->id = 999;
    stateTestCircle->alignH = common_renderer::HorizontalAlign::RIGHT;
    stateTestCircle->alignV = common_renderer::VerticalAlign::TOP;
    stateTestCircle->right = {10.0F, false};
    stateTestCircle->top = {10.0F, false};
    stateTestCircle->width = {20.0F, false};
    stateTestCircle->height = {20.0F, false};
    stateTestCircle->rotation.cx = 0.0F;
    stateTestCircle->rotation.cy = 0.0F;
    stateTestCircle->rotation.z = 0.0F;
    stateTestCircle->rotation.skewX = 0.0F;
    common_renderer::SolidColor stateTestColor;
    stateTestColor.color = "#00FF00"; // green
    stateTestCircle->fill = common_renderer::Fill(stateTestColor);
    stateTestCircle->clip = common_renderer::OvalClip{true};
    bool result = common_renderer::draw(testCanvas, stateTestCircle, testWidth, testHeight, false);
    return result;
  }

  // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
  float testWidth{};
  // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
  float testHeight{};
  // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
  SkCanvas *testCanvas{};
  // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
  sk_sp<SkSurface> testSurface;
};
