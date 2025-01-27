#pragma once

#include "FileFormats/BRDFile.h"

#include "imgui/imgui.h"
#include "imgui_operators.h"

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#define EMPTY_STRING ""
#define kBoardComponentPrefix "c_"
#define kBoardPinPrefix "p_"
#define kBoardNetPrefix "n_"
#define kBoardElementNameLength 127

using namespace std;

//struct Point;
struct BoardElement;
struct Net;
struct Pin;
struct Component;

typedef function<void(const char *)> TcharStringCallback;
typedef function<void(BoardElement *)> TboardElementCallback;


#if 0
template <typename T>
using obv_shared_ptr = std::shared_ptr<T>;
template <typename T, typename ...Args>
obv_shared_ptr<T> obv_make_shared(Args... args) { return std::make_shared<T>(args...); }
#else
template <typename T>
struct obv_shared_ptr {
	T * t_;
	obv_shared_ptr() { }
	explicit obv_shared_ptr(T * t) : t_(t) { }
	obv_shared_ptr(std::nullptr_t) : t_(nullptr) { }
	T & operator*() { return *t_; }
	T * operator->() { return t_; }
	T * get() { return t_; }
	const T & operator*() const { return *t_; }
	const T * operator->() const { return t_; }
	const T * get() const { return t_; }
	template <typename TT>
	operator obv_shared_ptr<TT>() { return obv_shared_ptr<TT>((TT *) get()); }
	bool operator==(std::nullptr_t) { return get() == nullptr; }
	bool operator!() const { return !t_; }
	operator bool() const { return t_; }
	void operator=(std::nullptr_t) { t_ = nullptr; }
	template <typename TT>
	void operator=(obv_shared_ptr<TT> p) { t_ = p.get(); }
    void operator=(T * t) { t_ = t; }
};

template <typename T>
bool operator==(obv_shared_ptr<T> const & a, obv_shared_ptr<T> const & b) {
	return a.get() == b.get();
}

template <typename T, typename ...Args>
obv_shared_ptr<T> obv_make_shared(Args... args) { return obv_shared_ptr<T>(new T(args...)); }
#endif

template <class T>
using SharedVector = vector<obv_shared_ptr<T>>;

template <class T>
using SharedStringMap = map<string, obv_shared_ptr<T>>;

enum EBoardSide {
	kBoardSideTop    = 0,
	kBoardSideBottom = 1,
	kBoardSideBoth   = 2,
};

// Checking whether str `prefix` is a prefix of str `base`.
inline static bool is_prefix(string prefix, string base) {
	if (prefix.size() > base.size()) return false;

	auto res = mismatch(prefix.begin(), prefix.end(), base.begin());
	if (res.first == prefix.end()) return true;

	return false;
}

template <class T>
inline static bool contains(const T &element, vector<T> &v) {
	return find(begin(v), end(v), element) != end(v);
}

template <class T>
inline static bool contains(T &element, vector<T> &v) {
	return find(begin(v), end(v), element) != end(v);
}

template <class T>
inline void remove(T &element, vector<T> &v) {

	auto it = std::find(v.begin(), v.end(), element);

	if (it != v.end()) {
		using std::swap;
		swap(*it, v.back());
		v.pop_back();
	}
}

struct BBox {
	ImVec2 min, max;

	ImVec2 dim() const {
		return max - min;
	}
	bool contains(ImVec2 const & c) {
		return c >= min && c < max;
	}
	bool contains(ImVec2 const & c, float sz) {
		return c.x + sz >= min.x && c.y + sz >= min.y
			&& c.x - sz < max.x && c.y - sz < max.y;
	}
};

// Any element being on the board.
struct BoardElement {
	// Side of the board the element is located. (top, bottom, both?)
	EBoardSide board_side = kBoardSideBoth;

	// String uniquely identifying this element on the board.
	virtual string UniqueId() const = 0;
	virtual ~BoardElement() { }
	
	void * tcl_priv = nullptr;
};

// A point/position on the board relative to top left corner of the board.
// TODO: not sure how different formats will store this info.


typedef ImVec2 Point;
#if 0
struct Point {
	float x, y;

	Point()
	    : x(0.0f)
	    , y(0.0f){};
	Point(float _x, float _y)
	    : x(_x)
	    , y(_y){};
	Point(int _x, int _y)
	    : x(float(_x))
	    , y(float(_y)){};
};
#endif

// Shared potential between multiple Pins/Contacts.
struct Net : BoardElement {
	int number;
	string name;
	bool is_ground;

	SharedVector<Pin> pins;

	string UniqueId() const {
		return kBoardNetPrefix + name;
	}
};

typedef ImVec2 outline_pt;
//struct outline_pt {
//	double x, y;
//};

// Any observeable contact (nails, component pins).
// Convieniently/Confusingly named Pin not Contact here.
struct Pin : BoardElement {
	enum EPinType {
		kPinTypeUnkown = 0,
		kPinTypeNotConnected,
		kPinTypeComponent,
		kPinTypeVia,
		kPinTypeTestPad,
	};

	// Type of Contact, e.g. pin, via, probe/test point.
	EPinType type;

	// Pin number / Nail count.
	string number;

	string name; // for BGA pads will be AZ82 etc

	// Position according to board file. (probably in inches)
	Point position;

	// Contact diameter, e.g. via or pin size. (probably in inches)
	float diameter;

	// Net this contact is connected to, nulltpr if no info available.
	Net *net;

	// Contact belonging to this component (pin), nullptr if nail.
	obv_shared_ptr<Component> component;

	string UniqueId() const {
		return kBoardPinPrefix + number;
	}
	float intensity_delta_ = 1.0f;
};

// A component on the board having multiple Pins.
struct Component : BoardElement {
	enum EMountType { kMountTypeUnknown = 0, kMountTypeSMD, kMountTypeDIP };

#if 0
	static const bool do_refcount = false;
#else
	static const bool do_refcount = true;
	static inline int icount = 0;
	Component() {
		++icount;
	}
	~Component() {
		--icount;
	}
#endif
	
	enum EComponentType {
		kComponentTypeUnknown = 0,
		kComponentTypeDummy,
		kComponentTypeConnector,
		kComponentTypeIC,
		kComponentTypeResistor,
		kComponentTypeCapacitor,
		kComponentTypeDiode,
		kComponentTypeTransistor,
		kComponentTypeCrystal,
		kComponentTypeJellyBean
	};

	// How the part is attached to the board, either SMD, .., through-hole?
	EMountType mount_type = kMountTypeUnknown;

	// Type of component, eg. resistor, cap, etc.
	EComponentType component_type = kComponentTypeUnknown;

	// Part name as stored in board file.
	string name;

	// Part manufacturing code (aka. part number).
	string mfgcode;

	// Pins belonging to this component.
	SharedVector<Pin> pins;

	// Post calculated outlines
	//
	outline_pt outline[4];
	Point p1{0.0f, 0.0f}, p2{0.0f, 0.0f}; // for debugging

	bool outline_done = false;
	outline_pt *hull  = NULL;
	int hull_count    = 0;
	ImVec2 omin, omax;
	ImVec2 centerpoint;
	double expanse = 0.0f; // quick measure of distance between pins.

	// enum ComponentVisualModes { CVMNormal = 0, CVMSelected, CVMShowPins, CVMModeCount };
	enum ComponentVisualModes { CVMNormal = 0, CVMSelected, CVMModeCount };

	uint8_t visualmode = 0;

	// Mount type as readable string.
	string mount_type_str() {
		switch (mount_type) {
			case Component::kMountTypeSMD: return "SMD";
			case Component::kMountTypeDIP: return "DIP";
			default: return "UNKNOWN";
		}
	}

	// true if component is not representing a real/physical component.
	bool is_dummy() {
		return component_type == kComponentTypeDummy;
	}

	string UniqueId() const {
		return kBoardComponentPrefix + name;
	}

	uint32_t shade_color_ = 0;
	float intensity_delta_ = 1;

};

struct Node {
	ImVec2 p1, p2;
	Net * net;
	bool top;
	bool stub;
};

class Board {
  public:
	enum EBoardType { kBoardTypeUnknown = 0, kBoardTypeBRD = 0x01, kBoardTypeBDV = 0x02 };

	virtual ~Board() {}

	virtual SharedVector<Node> &Nodes()           = 0;
	virtual SharedVector<Net> &Nets()             = 0;
	virtual SharedVector<Component> &Components() = 0;
	virtual SharedVector<Pin> &Pins()             = 0;
	virtual SharedVector<Point> &OutlinePoints()  = 0;

	EBoardType BoardType() {
		return kBoardTypeUnknown;
	}
};
