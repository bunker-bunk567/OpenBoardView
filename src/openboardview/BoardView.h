#pragma once

#include "Board.h"
#include "Searcher.h"
#include "SpellCorrector.h"
#include "annotations.h"
#include "confparse.h"
#include "history.h"
#include "imgui/imgui.h"
#include "UI/Keyboard/KeyBindings.h"
#include "GUI/Preferences/Keyboard.h"
#include "GUI/BackgroundImage.h"
#include "GUI/Preferences/BackgroundImage.h"
#include <cstdint>
#include <vector>
#include <mutex>

#define DPIF(x) (((x)*dpi) / 100.f)
#define DPI(x) (((x)*dpi) / 100)

struct BRDPart;
class BRDFile;
namespace OBV_Tcl {
	struct TCL;
}

struct BitVec {
	uint32_t *m_bits;
	// length of the BitVec in bits
	uint32_t m_size;

	~BitVec();
	void Resize(uint32_t new_size);

	bool operator[](uint32_t index) {
		return 0 != (m_bits[index >> 5] & (1u << (index & 0x1f)));
	}

	void Set(uint32_t index, bool val) {
		uint32_t &slot = m_bits[index >> 5];
		uint32_t bit   = (1u << (index & 0x1f));
		slot &= ~bit;
		if (val) {
			slot |= bit;
		}
	}

	void Clear() {
		uint32_t num_ints = m_size >> 5;
		for (uint32_t i = 0; i < num_ints; i++) {
			m_bits[i] = 0;
		}
	}
};

struct ColorScheme {
	/*
	 * Take note, because these are directly set
	 * the packing format is ABGR,  not RGBA
	 */
	uint32_t backgroundColor          = 0xffffffff;
	uint32_t boardFillColor           = 0xffdddddd;
	uint32_t partTextColor            = 0xff808000;
	uint32_t partTextBackgroundColor  = 0xff00eeee;
	uint32_t partOutlineColor         = 0xff444444;
	uint32_t partHullColor            = 0x80808080;
	uint32_t partFillColor            = 0xffffffff;
	uint32_t partHighlightedColor     = 0xff0000ee;
	uint32_t partHighlightedFillColor = 0xf4f0f0ff;
	uint32_t boardOutlineColor        = 0xff00ffff;

	//	uint32_t boxColor = 0xffcccccc;

	uint32_t pinDefaultColor      = 0xff0000ff;
	uint32_t pinDefaultTextColor  = 0xffcc0000;
	uint32_t pinGroundColor       = 0xff0000bb;
	uint32_t pinNotConnectedColor = 0xffff0000;
	uint32_t pinTestPadColor      = 0xff888888;
	uint32_t pinTestPadFillColor  = 0xff8dc6d6;
	uint32_t pinA1PadColor        = 0xffdd0000;

	uint32_t pinSelectedColor     = 0x00000000;
	uint32_t pinSelectedFillColor = 0xffff8888;
	uint32_t pinSelectedTextColor = 0xffffffff;

	uint32_t pinSameNetColor     = 0xffaa4040;
	uint32_t pinSameNetFillColor = 0xffff9999;
	uint32_t pinSameNetTextColor = 0xff111111;

	uint32_t pinHaloColor     = 0x8822FF22;
	uint32_t pinNetWebColor   = 0xff0000ff;
	uint32_t pinNetWebOSColor = 0x0000ff22;

	uint32_t annotationPartAliasColor       = 0xcc00ffff;
	uint32_t annotationBoxColor             = 0xaa0000ff;
	uint32_t annotationStalkColor           = 0xff000000;
	uint32_t annotationPopupBackgroundColor = 0xffeeeeee;
	uint32_t annotationPopupTextColor       = 0xff000000;

	uint32_t selectedMaskPins    = 0xFFFFFFFF;
	uint32_t selectedMaskParts   = 0xFFFFFFFF;
	uint32_t selectedMaskOutline = 0xFFFFFFFF;

	uint32_t orMaskPins    = 0x00000000;
	uint32_t orMaskParts   = 0x00000000;
	uint32_t orMaskOutline = 0x00000000;
};

// enum DrawChannel { kChannelImages = 0, kChannelFill, kChannelPolylines = 1, kChannelPins = 2, kChannelText = 3,
// kChannelAnnotations = 4, NUM_DRAW_CHANNELS = 5 };
enum DrawChannel {
	kChannelImages = 0,
	kChannelFill,
	kChannelPolylines,
	kChannelPins,
	kChannelText,
	kChannelAnnotations,
	NUM_DRAW_CHANNELS
};

enum FlipModes { flipModeVP = 0, flipModeMP = 1, NUM_FLIP_MODES };

struct BoardView {
	obv_shared_ptr<BRDFile> m_file;
	obv_shared_ptr<Board> m_board;
	BackgroundImage backgroundImage{m_current_side};

	Confparse obvconfig;
	FHistory fhistory;
	Searcher searcher;
	SpellCorrector scnets;
	SpellCorrector scparts;
	KeyBindings keybindings;
	KeyBindings * tcl_keybindings = nullptr;

	Preferences::Keyboard keyboardPreferences{keybindings, obvconfig};
	Preferences::BackgroundImage backgroundImagePreferences{keybindings, backgroundImage};
	Preferences::Keyboard * tcl_keyboardPreferences = nullptr;
	
	bool debug                   = false;
	int history_file_has_changed = 0;
	int dpi                      = 0;
	double dpiscale              = 1.0f;
	double fontSize              = 20.0f;
	float zoomFactor             = 0.5f;
	float partZoomScaleOutFactor = 3.0f;
	int zoomModifier             = 5;
	int panFactor                = 30;
	int panModifier              = 5;
	int flipMode                 = 0;
	bool startFullscreen         = false;

	bool dual_draw_side2 = false;
	ImVec2 dual_draw_offset;
	
	int annotationBoxOffset = 10;
	int annotationBoxSize   = 10;

	int pinA1threshold = 3; // pincount of package to show 1/A1 pin
	int netWebThickness = 2;

	float pinSizeThresholdLow = 0.0f;
	bool pinShapeSquare       = false;
	bool pinShapeCircle       = true;
	bool pinSelectMasks       = true;
	bool slowCPU              = false;
	bool showFPS              = false;
	bool showNetWeb           = true;
	bool showInfoPanel        = true;
	bool showPins             = true;
	bool showAnnotations      = true;
	bool pinHalo              = false;
	float pinHaloDiameter     = 1.1;
	float pinHaloThickness    = 4.00;
	bool fillParts            = true;
	bool boardFill            = true;
	int boardFillSpacing      = 3;
	bool boardMinMaxDone      = false;

	bool showPosition  = true;
	bool reloadConfig  = false;
	int pinBlank       = 0;
	uint32_t FZKey[44] = {0};

	int ConfigParse(void);
	static uint32_t byte4swap(uint32_t x);
	void CenterView(void);
	void Pan(int direction, int amount);
	void Zoom(float osd_x, float osd_y, float zoom);
	void Mirror(void);
	void DrawDiamond(ImDrawList *draw, ImVec2 c, double r, uint32_t color);
	void DrawHex(ImDrawList *draw, ImVec2 c, double r, uint32_t color);
	void DrawBox(ImDrawList *draw, ImVec2 c, double r, uint32_t color);
	void SetFZKey(const char *keytext);
	void HelpAbout(void);
	void HelpControls(void);
	template <class T>
	void ShowSearchResults(std::vector<T> results, char *search, int &limit, void (BoardView::*onSelect)(const char *));
	void SearchColumnGenerate(const std::string &title,
	                          std::pair<SharedVector<Component>, SharedVector<Net>> results,
	                          char *search,
	                          int limit);
	void Preferences(void);
	void SaveAllColors(void);
	void ColorPreferencesItem(
	    const char *label, int label_width, const char *butlabel, const char *conflabel, int var_width, uint32_t *c);
	void ColorPreferences(void);
	bool AnyItemVisible(void);
	void ThemeSetStyle(const char *name);

	bool infoPanelCenterZoomNets   = true;
	bool infoPanelSelectPartsOnNet = false;
	void CenterZoomNet(string netname);

	bool m_centerZoomSearchResults = true;
	void CenterZoomSearchResults(void);
	int EPCCheck(void);
	void OutlineGenFillDraw(ImDrawList *draw, int ydelta, double thickness);

	/* Context menu, sql stuff */
	Annotations m_annotations;
	void ContextMenu(void);
	int AnnotationIsHovered(void);
	bool AnnotationWasHovered     = false;
	bool m_annotationnew_retain   = false;
	bool m_annotationedit_retain  = false;
	bool m_tooltips_enabled       = true;
	bool m_parent_occluded        = false;
	int m_annotation_last_hovered = 0;
	int m_annotation_clicked_id   = 0;
	int m_hoverframes             = 0;
	ImVec2 m_previous_mouse_pos;

	/* Info/Side Pane */
	void ShowInfoPane(void);

	bool HighlightedPinIsHovered(void);
	obv_shared_ptr<Pin> m_pinHighlightedHovered    = nullptr;
	obv_shared_ptr<Pin> currentlyHoveredPin        = nullptr;
	obv_shared_ptr<Component> currentlyHoveredPart = nullptr;

	ImVec2 m_showContextMenuPos;

	obv_shared_ptr<Pin> m_pinSelected = nullptr;
	//	vector<Net *> m_netHiglighted;
	SharedVector<Pin> m_pinHighlighted;
	SharedVector<Component> m_partHighlighted;
	char m_cachedDrawList[sizeof(ImDrawList)];
	ImVector<char> m_cachedDrawCommands;
	SharedVector<Net> m_nets;
	char m_search[3][128];
	char m_netFilter[128];
	std::string m_lastFileOpenName;
	float m_dx; // display top-right coordinate?
	float m_dy;
	float m_mx; // board MID POINTS
	float m_my;
	float m_x_offset = 0;
	float m_y_offset = 0;
	bool  m_draw_both_sides = false;

	float m_split_view_x = 0.0f;
	float m_split_view_y = 0.0f;
	
	float m_scale       = 1.0f;
	float m_scale_floor = 1.0f; // scale which displays the entire board
	float m_lastWidth;          // previously checked on-screen window size; use to redraw
	                            // when window is resized?
	float m_lastHeight;
	int m_rotation; // set to 0 for original orientation [0-4]
	int m_current_side;
	int m_boardWidth; // board size in what coordinates? thou?
	int m_boardHeight;
	float m_menu_height;
	float m_status_height;
	ImVec2 m_board_surface;
	BBox m_board_surface_active;

	ImVec2 m_info_surface;
	int m_dragging_token = 0; // 1 = board window, 2 = side pane
	int m_tcl_drag = 0;

	static void wakeup();
	static int m_wakeup_pipe[2];
	
	ColorScheme m_colors;

	float m_default_intensity = 1.0;
	
	// TODO: save settings to disk
	// pinDiameter: diameter for all pins.  Unit scale: 1 = 0.025mm, boards are
	// done in "thou" (1/1000" = 0.0254mm)
	int m_pinDiameter     = 20;
	bool m_flipVertically = true;

	// Annotation layer specific
	bool m_annotationsVisible = true;

	// Do a hacky thing (memcpy the window draw list) to save us from rerendering
	// the board
	// The app will crash or break if this flag is not set when it should be.
	bool m_needsRedraw = true;
	bool m_draggingLastFrame;
	bool m_showContextMenu;
	//	bool m_showNetfilterSearch;
	bool m_showSearch;
	bool m_searchComponents = true;
	bool m_searchNets       = true;
	bool m_showNetList;
	bool m_showPartList;
	bool m_showHelpAbout;
	bool m_showHelpControls;
	bool m_showPreferences;
	bool m_showColorPreferences;
	bool m_firstFrame = true;
	bool m_lastFileOpenWasInvalid;
	bool m_validBoard = false;
	bool m_wantsQuit;

	std::mutex m_sleep_mutex;

	void sleep_mutex_lock() {
		m_sleep_mutex.lock();
	}
	void sleep_mutex_unlock() {
		m_sleep_mutex.unlock();
	}
	
	~BoardView();

	OBV_Tcl::TCL * m_tcl;
	void set_tcl(OBV_Tcl::TCL * t);
	SDL_Window * m_sdl_window = nullptr;
	bool m_is_fullscreen = false;
	void sdl_window(SDL_Window * w) {
		m_sdl_window = w;
		if (startFullscreen) {
			SDL_MaximizeWindow(m_sdl_window);
			m_is_fullscreen = true;
		}
	}

	std::mutex m_DrawNodes_mutex;
	
	void ShowNetList(bool *p_open);
	void ShowPartList(bool *p_open);

	void Update();
	void HandleInput();
	void RenderOverlay();
	void DrawPartTooltips(ImDrawList *draw);
	void DrawPinTooltips(ImDrawList *draw);
	void DrawAnnotations(ImDrawList *draw);
	void DrawOutline(ImDrawList *draw);
	void DrawNodes(ImDrawList * draw);
	void DrawPins(ImDrawList *draw);
	void DrawParts(ImDrawList *draw);
	bool DrawPartSymbol(ImDrawList * draw, Component * c);
	void DrawBoard();
	void DrawNetWeb(ImDrawList *draw);
	void SetFile(obv_shared_ptr<BRDFile> file, obv_shared_ptr<BRDBoard> board = nullptr);
	int LoadFile(const filesystem::path &filepath);
	BRDFile * loadBoard(const filesystem::path &filepath);
	ImVec2 CoordToScreen(float x, float y, float w = 1.0f);
	ImVec2 ScreenToCoord(float x, float y, float w = 1.0f);
	ImVec2 CoordToScreen(ImVec2 xy, float w = 1.0f) { return CoordToScreen(xy.x, xy.y, w); }
	ImVec2 ScreenToCoord(ImVec2 xy, float w = 1.0f) { return ScreenToCoord(xy.x, xy.y, w); }
	BBox   CoordToScreen(BBox const & box) {
		return { CoordToScreen(box.min), CoordToScreen(box.max) };
	}
	BBox   ScreenToCoord(BBox const & box) {
		ImVec2 p1 = ScreenToCoord(box.min), p2 = ScreenToCoord(box.max);
		return { { std::min(p1.x, p2.x), std::min(p1.y, p2.y) },
				{ std::max(p1.x, p2.x), std::max(p1.y, p2.y) } };
	}

	// void Move(float x, float y);
	void Rotate(int count);
	void DrawSelectedPins(ImDrawList *draw);
	void ClearAllHighlights(void);

	// Sets the center of the screen to (x,y) in board space
	void SetTarget(float x, float y);
	void SetTarget(ImVec2 xy) { SetTarget(xy.x, xy.y); }
	
	// Returns true if the part is shown on the currently displayed side of the
	// board.
	bool BoardElementIsVisible(const obv_shared_ptr<BoardElement> be);
	bool IsVisibleScreen(float x, float y, float radius, const ImGuiIO &io);
	// Returns true if the circle described by screen coordinates x, y, and radius
	// is visible in the
	// ImGuiIO screen rect.
	// bool IsVisibleScreen(float x, float y, float radius = 0.0f);

	bool PartIsHighlighted(const obv_shared_ptr<Component> component);
	void FindNet(const char *net);
	void FindNetNoClear(const char *name);
	void FindComponent(const char *name);
	void FindComponentNoClear(const char *name);
	void SearchComponent(void);
	void SearchNetNoClear(const char *net);
	void SearchCompound(const char *item);
	void SearchCompoundNoClear(const char *item);
	std::pair<SharedVector<Component>, SharedVector<Net>> SearchPartsAndNets(const char *search, int limit);

	void SetLastFileOpenName(const std::string &name);
	void FlipBoard(int mode = 0);
};
