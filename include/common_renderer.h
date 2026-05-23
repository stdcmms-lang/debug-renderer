#pragma once

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkPath.h"
#include "include/core/SkImage.h"
#include "include/core/SkPoint.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <array>
#include <variant>
#include <map>
#include <chrono>
#include <cstdint>

namespace common_renderer {

    // Font Manager Singleton
class FontManagerSingleton {
    public:
        // Get the singleton instance
        static FontManagerSingleton& getInstance();
        
        // Get the font manager
        sk_sp<SkFontMgr> getFontManager();
        
        // Set the font manager
        void setFontManager(sk_sp<SkFontMgr> fontMgr);
        
        // Reset the font manager (clears all state)
        void reset();
        
        // Delete copy constructor and assignment operator
        FontManagerSingleton(const FontManagerSingleton&) = delete;
        FontManagerSingleton& operator=(const FontManagerSingleton&) = delete;
        
        // Delete move constructor and move assignment operator
        FontManagerSingleton(FontManagerSingleton&&) = delete;
        FontManagerSingleton& operator=(FontManagerSingleton&&) = delete;
    
    private:
        FontManagerSingleton() = default;
        ~FontManagerSingleton() = default;
        
        sk_sp<SkFontMgr> fontManager;
        bool initialized = false;
    };
    
// Forward declarations
struct GradientColor;
struct Shadow;
struct Rotation;
struct Positioning;
struct Trim;
struct TextSection;
struct FontStyle;
struct AnyNode;

// Utility structures
struct Point {
    float x;
    float y;
};

// Position tuple: <value, isPercentage>
using PositionValue = std::tuple<float, bool>;

struct Rotation {
    float cx = 0.5F; // rotation center X in percentage relative to width, default to 0.5 (center)
    float cy = 0.5F; // rotation center Y in percentage relative to height, default to 0.5 (center)
    float z = 0.0F;  // rotation in degrees (clockwise)
    float skewX = 0.0F;
};

// Color types
struct Shadow {
    std::string type{}; // "dropShadow" or "innerShadow"
    float offsetX{};
    float offsetY{};
    float blurX{};
    float blurY{};
    std::string color{};
};

// Solid color representation
struct SolidColor {
    std::string color{};
    std::vector<Shadow> shadows{};  // Empty vector = no shadows
};

// Gradient color representation
struct GradientColor {
    std::string startColor{};
    std::string endColor{};
    Point startPoint{};
    Point endPoint{};
    std::vector<Shadow> shadows{};  // Empty vector = no shadows
};

// Color variant type
using Color = std::variant<SolidColor, GradientColor>;

// Image fill representation
struct ImageFill {
    sk_sp<SkImage> image;
    std::string fit{};
    std::optional<float> scaleBeforeRepeat;
    std::optional<float> rotateBeforeRepeat;
    std::optional<float> exposure;
    std::optional<float> contrast;
    std::optional<float> saturation;
    std::optional<float> temperature;
    std::optional<float> tint;
    std::optional<float> highlights;
    std::optional<float> shadows;
};

// Fill type that can be SolidColor, GradientColor, or ImageFill
using Fill = std::variant<SolidColor, GradientColor, ImageFill>;

// Positioning alignment constants
enum class HorizontalAlign : int {
    LEFT = 2,
    CENTER = 4,
    RIGHT = 6,
    EDGE = 8
};

enum class VerticalAlign : int {
    TOP = 1,
    CENTER = 3,
    BOTTOM = 5,
    EDGE = 7
};

enum class StrokeAlign : int {
    CENTER = 0,
    INSIDE = 1,
    OUTSIDE = 2
};

struct Positioning {
    HorizontalAlign alignH{HorizontalAlign::LEFT};
    VerticalAlign alignV{VerticalAlign::TOP};
    std::optional<PositionValue> left;
    std::optional<PositionValue> right;
    std::optional<PositionValue> top;
    std::optional<PositionValue> bottom;
    std::optional<PositionValue> width;
    std::optional<PositionValue> height;
    
    // Constructor with default initialization
    Positioning()  = default;
};

struct Trim {
    float start = 0.0F; // 0.0 to 1.0
    float end = 1.0F;   // 0.0 to 1.0
};

struct RRectClip {
    std::array<float, 4> r = {0.0F, 0.0F, 0.0F, 0.0F};  // Corner radii (for rrect): [top-left, top-right, bottom-right, bottom-left]
    bool enable = false;    // Whether to clip content
};

struct OvalClip {
    bool enable = false;    // Whether to clip content
};

struct PathClip {
    SkPath path;           // Path data (for path type)
    bool enable = false;    // Whether to clip content
};

// Clip variant type
using Clip = std::variant<RRectClip, OvalClip, PathClip>;

// Text-related structures

struct FontStyle {
    int weight;
    SkFontStyle::Slant slant;
};

// Text decoration types
struct TextDecoration {
    skia::textlayout::TextDecoration type = skia::textlayout::TextDecoration::kNoDecoration;
    std::string color = "#000000";
    float thickness = 1.0F;
    skia::textlayout::TextDecorationStyle style = skia::textlayout::TextDecorationStyle::kSolid;
    
    // Default constructor
    TextDecoration() = default;
};

struct TextSection {
    std::string txt{};
    std::string fontFamily{};
    float fontSize = 16.0F;
    FontStyle fontStyle{};
    std::optional<Color> foregroundColor;
    float letterSpacing = 0.0F;
    float wordSpacing = 0.0F;
    float lineHeight = 1.0F;
    std::optional<TextDecoration> decoration = std::nullopt;
    
    // Default constructor
    TextSection() = default;
};

struct Stroke {
    float width = 1.0F;
    std::optional<Color> color;
    StrokeAlign align = StrokeAlign::CENTER;
};

// Effect types
struct Blur {
    float blur = 0.0F;
};

struct DisplacementMap {
    float amplitude = 0.0F;
    float freqX = 0.0F;
    float freqY = 0.0F;
    int octaves = 1;
};

struct CRTEffect {
    float verticalBleed = 0.5F;
    float horizontalBleed = 0.5F;
    float contrast = 0.5F; // 0-1
    float granularity = 2.0F;
};

struct LiquidGlass {
    float refraction = 1.5F; // 1-2
    float tint = 0.0F; // 0-1
    float rim = 10.0F;
    float cornerRadius = 0.0F; // Corner radius
};

// Effect variant type
using Effect = std::variant<Blur, DisplacementMap, CRTEffect, LiquidGlass>;

enum class TransitionType: int {
    INSTANT = 1,
    MOVEIN = 2,
    MOVEOUT = 3,
};

enum class TransitionDirection: int {
    LEFT = 1,
    RIGHT = 2,
    UP = 3,
    DOWN = 4,
};

// Node types
enum class NodeType {
    PARAGRAPH,
    LAYER,
    SCREEN,
    SVG,
    PATH,
    ROOT
};

// Base node structure
struct AnyNode {
    NodeType type{NodeType::LAYER};
    std::string name{};
    int id{};
    
    // Default constructor
    AnyNode()  = default;
    
    // Copy constructor
    AnyNode(const AnyNode&) = default;
    
    // Copy assignment operator
    AnyNode& operator=(const AnyNode&) = default;
    
    // Move constructor
    AnyNode(AnyNode&&) = default;
    
    // Move assignment operator
    AnyNode& operator=(AnyNode&&) = default;
    
    // Virtual destructor for proper inheritance
    virtual ~AnyNode() = default;
};

// Specialized node types with their specific fields
struct ParagraphNode : public AnyNode, Positioning {
    // Common fields for all nodes except Image
    Rotation rotation{};
    
    // Fields specific to Paragraph
    skia::textlayout::TextAlign textAlign{skia::textlayout::TextAlign::kLeft};
    bool textOverflow{false}; // true = wrap, false = clip
    std::vector<TextSection> texts{};
    std::optional<int> cursorOffset; // UTF-16 offset for cursor position in EditText mode

    ParagraphNode()  {
        type = NodeType::PARAGRAPH;
    }
};

struct LayerNode : public AnyNode, Positioning {
    // Common fields for all nodes except Image
    Rotation rotation{};
    
    // Fields specific to Layer
    std::optional<Stroke> stroke;
    std::optional<Fill> fill;
    std::vector<std::shared_ptr<AnyNode>> children{};
    std::optional<Clip> clip;
    std::optional<Trim> trim;
    std::vector<Effect> foregroundEffects{};
    std::vector<Effect> backdropEffects{};
    std::optional<float> opacity;

    LayerNode() {
        type = NodeType::LAYER;
    }
};

struct ScreenNode : public AnyNode, Positioning {
    // Common fields for all nodes except Image    
    // Fields specific to Screen
    std::optional<Stroke> stroke;
    std::optional<Fill> fill;
    std::vector<std::shared_ptr<AnyNode>> children{};
    std::optional<Clip> clip;

    ScreenNode() {
        type = NodeType::SCREEN;
    }
};

struct SVGNode : public AnyNode, Positioning {
    // Common fields for all nodes except Image
    Rotation rotation{};
    
    // Fields specific to SVG
    std::string svg{};

    SVGNode() {
        type = NodeType::SVG;
    }
};

struct PathNode : public AnyNode, Positioning {
    // Common fields for all nodes except Image
    Rotation rotation{};
    
    // Fields specific to Path
    SkPath path;
    std::optional<Stroke> stroke;
    std::optional<Fill> fill;
    std::optional<Trim> trim;

    PathNode() {
        type = NodeType::PATH;
    }
};

struct RootNode : public AnyNode {
    std::tuple<float, float> pan = std::make_tuple(0.0F, 0.0F);
    std::tuple<float, float> scale = std::make_tuple(1.0F, 1.0F);
    std::tuple<float, float> origin = std::make_tuple(0.0F, 0.0F);
    std::vector<std::shared_ptr<AnyNode>> children{};

    RootNode() {
        type = NodeType::ROOT;
    }
};


bool drawNode(SkCanvas* canvas, const std::shared_ptr<AnyNode>& node, float offsetX, float offsetY, float parentWidth, float parentHeight);

/**
 * @brief Draws a node on the provided Skia canvas
 * 
 * This function is the main entry point for rendering a node. It
 * dispatches to the appropriate renderer based on the node type.
 * 
 * @param canvas The Skia canvas to draw on
 * @param node The node to draw
 * @param canvasWidth The logical width of the canvas, this may be different from canvas->getBaseLayerSize() because of device pixel ratio
 * @param canvasHeight The logical height of the canvas
 * @param clearCanvas Whether to clear the canvas before drawing (default: true)
 */
bool draw(SkCanvas* canvas, const std::shared_ptr<AnyNode>& rootNode, float canvasWidth, float canvasHeight, bool clearCanvas = true);

/**
 * @brief Draws a transition animation between two screen images
 * 
 * This function renders a slide transition animation using a custom shader that blends
 * between a source image and a target image based on progress and direction.
 * 
 * @param canvas The canvas to draw on
 * @param sourceImage The source screen image (current screen)
 * @param targetImage The target screen image (new screen)
 * @param progress Transition progress from 0.0 to 1.0
 * @param animationType The type of animation (INSTANT, MOVEIN, MOVEOUT)
 * @param directionX X component of the slide direction vector (normalized)
 * @param directionY Y component of the slide direction vector (normalized)
 * @param canvasWidth Width of the canvas in logical pixels
 * @param canvasHeight Height of the canvas in logical pixels
 * @param dpr Device pixel ratio (used to scale image shader coordinates)
 * @return true on success, false on failure
 */
bool drawTransition(SkCanvas* canvas, sk_sp<SkImage> sourceImage, sk_sp<SkImage> targetImage, float progress, TransitionType animationType, float directionX, float directionY, float canvasWidth, float canvasHeight, float dpr);

/**
 * @brief Creates a snapshot of the surface and returns the PNG bytes
 * 
 * This function takes a snapshot of the entire surface and returns it as raw PNG bytes.
 * 
 * @param surface The Skia surface to snapshot from
 * @param outPtr Output parameter: pointer to malloc-ed PNG bytes (caller must free with free())
 * @return Size of the PNG data, or 0 on failure
 */
size_t makeSnapshot(SkSurface* surface, uint8_t** outPtr);

/**
 * @brief Converts TransitionDirection to a normalized direction vector
 * 
 * @param dir The transition direction
 * @return SkPoint representing the normalized direction vector
 */
SkPoint getDirectionVector(TransitionDirection dir);

/**
 * @brief Gets the current time in milliseconds
 * 
 * Platform-agnostic implementation using std::chrono.
 * 
 * @return Current time in milliseconds since epoch
 */
int64_t getCurrentTimeMs();

/**
 * @brief Initializes screen nodes from design state JSON strings
 * 
 * Parses the default design state JSON strings and populates the screen nodes map.
 * 
 * @param designStateStrings Vector of JSON strings, one per screen
 * @param logicalWidth Logical width of the screen
 * @param logicalHeight Logical height of the screen
 * @param screenNodesMap Output parameter: map of screen IDs to ScreenNode pointers
 * @return The ID of the first screen, or -1 if no screens were parsed
 */
int initializeScreenNodes(
    const std::vector<const char*>& designStateStrings,
    float logicalWidth,
    float logicalHeight,
    std::map<int, std::shared_ptr<ScreenNode>>& screenNodesMap);

} // namespace common_renderer
