#pragma once

#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkPaint.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/effects/SkRuntimeEffect.h"
#include "common_renderer.h"
#include <array>
#include <memory>
#include <optional>
#include <vector>
#include <string>


namespace skia::textlayout {
class Paragraph;
}


namespace common_renderer {

/**
 * @brief Calculates top, left, width, and height positioning for elements
 * 
 * This function converts positioning data from a JSON-like structure to actual
 * pixel coordinates and dimensions based on alignment and percentage values.
 * 
 * @param jsonPtr The node containing positioning information
 * @param parentWidth The width of the parent container
 * @param parentHeight The height of the parent container
 * @return A vector containing [x, y, width, height] coordinates
 */
std::vector<float> toTlwhPriorRotation(const std::shared_ptr<AnyNode>& jsonPtr, float parentWidth, float parentHeight);

/**
 * @brief Parses a color string to SkColor
 * 
 * This function converts color strings (hex format) to SkColor values.
 * Supports both RGB (#RRGGBB) and RGBA (#RRGGBBAA) formats.
 * 
 * @param colorStr The color string to parse
 * @return The parsed SkColor, or SK_ColorBLACK if parsing fails
 */
SkColor parseColor(const std::string& colorStr);

/**
 * @brief Creates SkPaint from Color variant
 * 
 * This function creates a SkPaint object from a Color variant, handling both
 * solid colors and gradients. For gradients, it creates a linear gradient shader.
 * Additionally, it supports drop shadow effects when specified in the Color variant.
 * 
 * @param color The Color variant to convert
 * @param offsetX X offset of the element
 * @param offsetY Y offset of the element
 * @param width The width of the element
 * @param height The height of the element
 * @return A SkPaint object with appropriate color, gradient shader, and drop shadow effects
 */
SkPaint createPaintFromColor(const std::optional<Color>& color, float offsetX, float offsetY, float width, float height);

/**
 * @brief Creates SkPaint from Fill variant
 * 
 * This function creates a SkPaint object from a Fill variant, handling
 * SolidColor, GradientColor, or ImageFill. For gradients, it creates a linear
 * gradient shader. For image fills, it creates an image shader with cover fit.
 * Additionally, it supports drop shadow effects when specified.
 * 
 * @param fill The Fill variant to convert
 * @param offsetX X offset of the element
 * @param offsetY Y offset of the element
 * @param width The width of the element
 * @param height The height of the element
 * @return A SkPaint object with appropriate color, gradient shader, image shader, and drop shadow effects
 */
SkPaint createPaintFromFill(const Fill& fill, float offsetX, float offsetY, float width, float height);


/**
 * @brief Loads an image from a data URL with caching
 * 
 * This function loads an image from a data URL and returns a SkImage object.
 * Images are cached using a hash-based cache for improved performance.
 * 
 * @param imageData The data URL of the image to load
 * @return A SkImage object, or nullptr if loading fails
 */
sk_sp<SkImage> loadImageFromData(const std::string& imageData);

/**
 * @brief Clears the image cache
 * 
 * This function clears all cached images, freeing memory.
 */
void clearImageCache();

/**
 * @brief Creates an ImageFilter from a vector of effects
 * 
 * This function processes a vector of effects (Blur, DisplacementMap, etc.) and
 * combines them into a single ImageFilter. Effects are applied in order, with
 * each effect chaining onto the previous one.
 * 
 * @param effects Vector of effects to process
 * @param offsetX The X offset/position of the image/layer
 * @param offsetY The Y offset/position of the image/layer
 * @param width The width of the image/layer to apply effects to
 * @param height The height of the image/layer to apply effects to
 * @param cornerRadii The corner radii from clip [top-left, top-right, bottom-right, bottom-left]
 * @return A combined SkImageFilter, or nullptr if no effects are provided or all effects fail
 */
sk_sp<SkImageFilter> createImageFilterFromEffects(const std::vector<Effect>& effects, float offsetX, float offsetY, float width, float height, const std::array<float, 4>& cornerRadii = {0.0F, 0.0F, 0.0F, 0.0F});

/**
 * @brief Creates an ImageFilter for inner shadow effect
 * 
 * This function creates an image filter that produces an inner shadow effect by:
 * 1. Creating source graphic and source alpha filters
 * 2. Applying color blend with offset and blur
 * 3. Composing the result with the input filter
 * 
 * @param input The input image filter to compose with
 * @param color The shadow color
 * @param dx The horizontal offset of the shadow
 * @param dy The vertical offset of the shadow
 * @param sigmaX The horizontal blur radius (sigma)
 * @param sigmaY The vertical blur radius (sigma)
 * @return A composed SkImageFilter that produces an inner shadow effect
 */
sk_sp<SkImageFilter> createImageFilterForInnerShadow(sk_sp<SkImageFilter> input, SkColor color, float dx, float dy, float sigmaX, float sigmaY);

/**
 * @brief Checks if clip is enabled
 * 
 * @param clip The clip configuration
 * @return true if clip is enabled, false otherwise
 */
bool isClipEnabled(const Clip& clip);

/**
 * @brief Checks if an effect should be disabled based on clip state
 * 
 * @param effect The effect to check
 * @param clip The clip configuration
 * @param isBackdrop Whether this is a backdrop effect (true) or foreground effect (false)
 * @return true if the effect should be disabled, false otherwise
 */
bool isEffectDisabled(const Effect& effect, const Clip& clip, bool isBackdrop);

/**
 * @brief Filters effects based on clip state
 * 
 * @param effects Vector of effects to filter
 * @param clip The clip configuration
 * @param isBackdrop Whether these are backdrop effects (true) or foreground effects (false)
 * @return Filtered vector of effects (disabled effects removed)
 */
std::vector<Effect> filterEffects(const std::vector<Effect>& effects, const Clip& clip, bool isBackdrop);

/**
 * @brief Adjusts bounds for stroke alignment
 * 
 * This function adjusts the bounds of a rectangle based on stroke alignment.
 * For INSIDE stroke alignment, the bounds are inset by half the stroke width.
 * For OUTSIDE stroke alignment, the bounds are outset by half the stroke width.
 * For CENTER stroke alignment, the bounds remain unchanged.
 * 
 * @param bounds The original bounds rectangle
 * @param strokeAlign The stroke alignment (CENTER, INSIDE, or OUTSIDE)
 * @param strokeWidth The width of the stroke
 * @return Adjusted bounds rectangle
 */
SkRect adjustBoundsForStroke(const SkRect& bounds, StrokeAlign strokeAlign, float strokeWidth);

/**
 * @brief Extracts corner radii from a clip path
 * 
 * This function extracts corner radii from a clip path. If the path is an oval,
 * it returns -1 for all corners to indicate no corner radii. If the path is an
 * RRect, it returns the effective radii for each corner. Otherwise, it returns zeros.
 * 
 * @param clipPath The clip path to extract corner radii from
 * @return Array of corner radii [top-left, top-right, bottom-right, bottom-left], or -1 for all if oval
 */
std::array<float, 4> getCornerRadiiFromClip(const SkPath& clipPath);

/**
 * @brief Gets a clip path based on clip type
 * 
 * This function creates a SkPath based on the clip configuration. It supports
 * RRectClip (rounded rectangles), OvalClip (ellipses/ovals), and PathClip (custom paths).
 * 
 * @param clip The clip configuration
 * @param bounds The bounds rectangle for the clip
 * @return A SkPath representing the clip shape
 */
SkPath getClipPath(const Clip& clip, const SkRect& bounds);

/**
 * @brief Renders a text cursor at the specified UTF-16 offset in a paragraph
 * 
 * This function renders a vertical line cursor at the specified cursor offset in the paragraph.
 * It handles edge cases like cursor at the start (offset 0) or at the end of text.
 * The cursor position is calculated based on glyph bounds and text direction (LTR/RTL).
 * 
 * @param paragraph The paragraph object containing the text
 * @param cursorOffset The UTF-16 offset where the cursor should be rendered
 * @param x The X coordinate of the paragraph's top-left corner
 * @param y The Y coordinate of the paragraph's top-left corner
 * @param canvas The canvas to draw the cursor on
 */
void renderParagraphCursor(skia::textlayout::Paragraph* paragraph, size_t cursorOffset, float x, float y, SkCanvas* canvas);

/**
 * @brief Gets the cached MoveIn shader effect
 * 
 * Target slides in from the specified direction, source stays put.
 * The shader effect is compiled once and reused for performance.
 * 
 * @return A cached SkRuntimeEffect for move-in transitions, or nullptr if compilation fails
 */
sk_sp<SkRuntimeEffect> getMoveInShaderEffect();

/**
 * @brief Gets the cached MoveOut shader effect
 * 
 * Source slides out in the specified direction, target stays put.
 * The shader effect is compiled once and reused for performance.
 * 
 * @return A cached SkRuntimeEffect for move-out transitions, or nullptr if compilation fails
 */
sk_sp<SkRuntimeEffect> getMoveOutShaderEffect();

/**
 * @brief Finds a node by ID in the node tree recursively
 * 
 * This function searches through the node tree starting from the root node
 * to find a node with the specified ID.
 * 
 * @param root The root node to start searching from
 * @param id The node ID to search for
 * @return A shared pointer to the found node, or nullptr if not found
 */
std::shared_ptr<AnyNode> findNodeById(const std::shared_ptr<AnyNode>& root, int id);

} // namespace common_renderer
