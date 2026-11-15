#pragma once
//v1
// draggable 2d gizmos for Dear ImGui. Handles, rects, circles, etc.
// Just drop this header in and you're good to go.
//
// Quick start:
//
//   #include "ImGizmo2D.h"
//
//   // hook it up to your viewport or screen
//   ImGizmo2D::SetDrawList(ImGui::GetWindowDrawList());
//   ImGizmo2D::SetViewRect(canvasPos, canvasSize);
//   ImGizmo2D::SetViewTransform(camX, camY, zoom);
//   ImGizmo2D::BeginFrame();
//
//   // now just use whatever you need
//   ImGizmo2D::Translate("player", &pos.x, &pos.y);
//   ImGizmo2D::Rect("hitbox", &x, &y, &w, &h);
//   ImGizmo2D::Circle("range", &cx, &cy, &radius);
//   ImGizmo2D::Rotate("turret", &cx, &cy, &angle);
//   ImGizmo2D::Scale("sprite", &x, &y, &sx, &sy);
//   ImGizmo2D::Point("waypoint", &px, &py);
//   ImGizmo2D::Line("wall", &x1, &y1, &x2, &y2);
//   ImGizmo2D::Polygon("zone", points, count);
//
// https://github.com/Miisan-png/ImGizmo2D :PP 

#include <imgui.h>
#include <cmath>
#include <cstdio>

namespace ImGizmo2D {

// fnv-1a hash so each handle gets a unique id even across gizmos
inline ImGuiID HashID(const char* id, int handle) {
    ImGuiID h = 2166136261u;
    for (const char* p = id; *p; p++) {
        h ^= (ImGuiID)*p;
        h *= 16777619u;
    }
    h ^= (ImGuiID)(handle * 2654435761u);
    return h;
}

struct Context {
    ImDrawList* drawList = nullptr;
    ImVec2 viewOrigin{0, 0};
    ImVec2 viewSize{0, 0};
    ImVec2 cameraPos{0, 0};
    float zoom = 1.0f;

    float handleRadius = 6.0f;
    float lineThickness = 2.0f;
    float snapGrid = 0.0f;

    ImU32 colIdle      = IM_COL32(100, 200, 220, 255);
    ImU32 colHover     = IM_COL32(255, 220, 80, 255);
    ImU32 colActive    = IM_COL32(255, 255, 120, 255);
    ImU32 colLine      = IM_COL32(100, 200, 220, 180);
    ImU32 colFill      = IM_COL32(100, 200, 220, 30);
    ImU32 colAxisX     = IM_COL32(230, 70, 70, 255);
    ImU32 colAxisY     = IM_COL32(70, 200, 70, 255);

    ImGuiID activeId = 0;
    ImGuiID hoveredId = 0;
    const char* activeName = nullptr;
    const char* hoveredName = nullptr;

    ImVec2 grabOffset{0, 0}; // so handles don't snap-jump to cursor on click
};

inline Context& GetContext() {
    static Context ctx;
    return ctx;
}

inline void SetDrawList(ImDrawList* dl) { GetContext().drawList = dl; }

inline void SetViewRect(ImVec2 origin, ImVec2 size) {
    auto& ctx = GetContext();
    ctx.viewOrigin = origin;
    ctx.viewSize = size;
}

inline void SetViewTransform(float camX, float camY, float zoom) {
    auto& ctx = GetContext();
    ctx.cameraPos = {camX, camY};
    ctx.zoom = zoom;
}

inline void SetSnapGrid(float snap) { GetContext().snapGrid = snap; }
inline void SetHandleRadius(float r) { GetContext().handleRadius = r; }

inline void SetColors(ImU32 idle, ImU32 hover, ImU32 active) {
    auto& ctx = GetContext();
    ctx.colIdle = idle;
    ctx.colHover = hover;
    ctx.colActive = active;
    ctx.colLine = idle;
}

inline void SetLineColor(ImU32 col) { GetContext().colLine = col; }
inline void SetFillColor(ImU32 col) { GetContext().colFill = col; }
inline void SetAxisColors(ImU32 x, ImU32 y) { GetContext().colAxisX = x; GetContext().colAxisY = y; }
inline void SetLineThickness(float t) { GetContext().lineThickness = t; }

// call once per frame before any gizmos. resets hover, releases active on mouse up.
inline void BeginFrame() {
    auto& ctx = GetContext();
    ctx.hoveredId = 0;
    ctx.hoveredName = nullptr;
    if (!ImGui::IsMouseDown(0)) {
        ctx.activeId = 0;
        ctx.activeName = nullptr;
    }
}

inline ImVec2 WorldToScreen(float wx, float wy) {
    auto& ctx = GetContext();
    float sx = ctx.viewOrigin.x + (wx - ctx.cameraPos.x) * ctx.zoom + ctx.viewSize.x * 0.5f;
    float sy = ctx.viewOrigin.y + (wy - ctx.cameraPos.y) * ctx.zoom + ctx.viewSize.y * 0.5f;
    return {sx, sy};
}

inline ImVec2 ScreenToWorld(float sx, float sy) {
    auto& ctx = GetContext();
    float wx = (sx - ctx.viewOrigin.x - ctx.viewSize.x * 0.5f) / ctx.zoom + ctx.cameraPos.x;
    float wy = (sy - ctx.viewOrigin.y - ctx.viewSize.y * 0.5f) / ctx.zoom + ctx.cameraPos.y;
    return {wx, wy};
}

inline float Snap(float v, float grid) {
    if (grid <= 0) return v;
    return std::round(v / grid) * grid;
}

// the core of everything - a single draggable point. all gizmos are built from these.
inline bool HandlePoint(const char* parentId, int handle, float* x, float* y) {
    auto& ctx = GetContext();
    ImGuiID id = HashID(parentId, handle);

    ImVec2 sp = WorldToScreen(*x, *y);
    ImVec2 mouse = ImGui::GetMousePos();
    float dx = mouse.x - sp.x;
    float dy = mouse.y - sp.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    bool hovered = dist <= ctx.handleRadius + 3.0f;
    bool isActive = (ctx.activeId == id);
    bool modified = false;

    if (hovered && ctx.activeId == 0) {
        ctx.hoveredId = id;
        ctx.hoveredName = parentId;
    }

    if (hovered && ImGui::IsMouseClicked(0) && ctx.activeId == 0) {
        ctx.activeId = id;
        ctx.activeName = parentId;
        ImVec2 wp = ScreenToWorld(mouse.x, mouse.y);
        ctx.grabOffset = {wp.x - *x, wp.y - *y};
    }

    if (isActive && ImGui::IsMouseDown(0)) {
        ImVec2 w = ScreenToWorld(mouse.x, mouse.y);
        *x = Snap(w.x - ctx.grabOffset.x, ctx.snapGrid);
        *y = Snap(w.y - ctx.grabOffset.y, ctx.snapGrid);
        modified = true;
    }

    ImU32 col = isActive ? ctx.colActive : ((ctx.hoveredId == id) ? ctx.colHover : ctx.colIdle);
    ctx.drawList->AddCircleFilled(sp, ctx.handleRadius, col);
    ctx.drawList->AddCircle(sp, ctx.handleRadius, IM_COL32(0, 0, 0, 180), 0, 1.5f);

    return modified;
}

// --- gizmos -----------------------------------------------------------------

// like HandlePoint but locked to one axis. used by Translate and Scale.
inline bool HandleAxis(const char* parentId, int handle, float* val, float wx, float wy, bool isX) {
    auto& ctx = GetContext();
    ImGuiID hid = HashID(parentId, handle);

    ImVec2 sp = WorldToScreen(wx, wy);
    ImVec2 mouse = ImGui::GetMousePos();

    float hitDist = isX ? std::abs(mouse.y - sp.y) : std::abs(mouse.x - sp.x);
    float hitAlong;
    ImVec2 origin = WorldToScreen(wx - (isX ? (wx - *val + wx) * 0 : 0), wy);
    origin = WorldToScreen(isX ? *val : wx, isX ? wy : *val);

    float dx = mouse.x - sp.x;
    float dy = mouse.y - sp.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    bool hovered = dist <= ctx.handleRadius + 4.0f;
    bool isActive = (ctx.activeId == hid);
    bool modified = false;

    if (hovered && ctx.activeId == 0) {
        ctx.hoveredId = hid;
        ctx.hoveredName = parentId;
    }

    if (hovered && ImGui::IsMouseClicked(0) && ctx.activeId == 0) {
        ctx.activeId = hid;
        ctx.activeName = parentId;
        ImVec2 wp = ScreenToWorld(mouse.x, mouse.y);
        ctx.grabOffset = {isX ? wp.x - *val : 0, isX ? 0 : wp.y - *val};
    }

    if (isActive && ImGui::IsMouseDown(0)) {
        ImVec2 w = ScreenToWorld(mouse.x, mouse.y);
        if (isX)
            *val = Snap(w.x - ctx.grabOffset.x, ctx.snapGrid);
        else
            *val = Snap(w.y - ctx.grabOffset.y, ctx.snapGrid);
        modified = true;
    }

    ImU32 col = isActive ? ctx.colActive : ((ctx.hoveredId == hid) ? ctx.colHover : (isX ? ctx.colAxisX : ctx.colAxisY));
    ctx.drawList->AddCircleFilled(sp, ctx.handleRadius - 1, col);
    ctx.drawList->AddCircle(sp, ctx.handleRadius - 1, IM_COL32(0, 0, 0, 180), 0, 1.5f);

    return modified;
}

// move something. center handle = free, arrows = axis-locked.
inline bool Translate(const char* id, float* x, float* y) {
    auto& ctx = GetContext();
    float axisLen = 40.0f;
    bool modified = false;

    ImVec2 sp = WorldToScreen(*x, *y);
    ImVec2 axTip{sp.x + axisLen, sp.y};
    ImVec2 ayTip{sp.x, sp.y + axisLen};

    ctx.drawList->AddLine(sp, axTip, ctx.colAxisX, ctx.lineThickness);
    ctx.drawList->AddLine(sp, ayTip, ctx.colAxisY, ctx.lineThickness);

    ctx.drawList->AddTriangleFilled(
        {axTip.x + 6, axTip.y}, {axTip.x - 2, axTip.y - 5}, {axTip.x - 2, axTip.y + 5}, ctx.colAxisX);
    ctx.drawList->AddTriangleFilled(
        {ayTip.x, ayTip.y + 6}, {ayTip.x - 5, ayTip.y - 2}, {ayTip.x + 5, ayTip.y - 2}, ctx.colAxisY);

    float axHandleX = *x + axisLen / ctx.zoom;
    float ayHandleY = *y + axisLen / ctx.zoom;

    if (HandleAxis(id, 1, x, axHandleX, *y, true)) {
        modified = true;
    }

    if (HandleAxis(id, 2, y, *x, ayHandleY, false)) {
        modified = true;
    }

    modified |= HandlePoint(id, 0, x, y);

    return modified;
}

// resizable box. drag any corner to resize, keeps min 1x1.
inline bool Rect(const char* id, float* x, float* y, float* w, float* h) {
    auto& ctx = GetContext();
    bool modified = false;

    ImVec2 tl = WorldToScreen(*x, *y);
    ImVec2 br = WorldToScreen(*x + *w, *y + *h);

    ctx.drawList->AddRectFilled(tl, br, ctx.colFill);
    ctx.drawList->AddRect(tl, br, ctx.colLine, 0, 0, ctx.lineThickness);

    float corners[4][2] = {
        {*x, *y},
        {*x + *w, *y},
        {*x, *y + *h},
        {*x + *w, *y + *h},
    };

    for (int i = 0; i < 4; i++) {
        float px = corners[i][0], py = corners[i][1];
        if (HandlePoint(id, i + 1, &px, &py)) {
            modified = true;
            switch (i) {
                case 0: *w += *x - px; *h += *y - py; *x = px; *y = py; break;
                case 1: *w = px - *x; *h += *y - py; *y = py; break;
                case 2: *w += *x - px; *x = px; *h = py - *y; break;
                case 3: *w = px - *x; *h = py - *y; break;
            }
        }
    }

    if (*w < 1.0f) *w = 1.0f;
    if (*h < 1.0f) *h = 1.0f;

    return modified;
}

// center + edge handle to drag the radius
inline bool Circle(const char* id, float* cx, float* cy, float* radius) {
    auto& ctx = GetContext();
    bool modified = false;

    ImVec2 center = WorldToScreen(*cx, *cy);
    float screenR = *radius * ctx.zoom;

    ctx.drawList->AddCircleFilled(center, screenR, ctx.colFill);
    ctx.drawList->AddCircle(center, screenR, ctx.colLine, 0, ctx.lineThickness);

    modified |= HandlePoint(id, 0, cx, cy);

    float edgeX = *cx + *radius;
    float edgeY = *cy;
    if (HandlePoint(id, 1, &edgeX, &edgeY)) {
        float ddx = edgeX - *cx;
        float ddy = edgeY - *cy;
        *radius = std::sqrt(ddx * ddx + ddy * ddy);
        if (*radius < 1.0f) *radius = 1.0f;
        modified = true;
    }

    return modified;
}

// angle in degrees. drag the handle around the ring.
inline bool Rotate(const char* id, float* cx, float* cy, float* angle) {
    auto& ctx = GetContext();
    bool modified = false;

    ImVec2 center = WorldToScreen(*cx, *cy);
    float ringR = 50.0f;

    ctx.drawList->AddCircle(center, ringR, ctx.colLine, 0, ctx.lineThickness);

    float rad = *angle * 3.14159265f / 180.0f;
    float hx = center.x + std::cos(rad) * ringR;
    float hy = center.y + std::sin(rad) * ringR;

    ctx.drawList->AddLine(center, {hx, hy}, ctx.colActive, ctx.lineThickness + 1);

    ImGuiID handleId = HashID(id, 1);
    ImVec2 mouse = ImGui::GetMousePos();
    float dist = std::sqrt((mouse.x - hx) * (mouse.x - hx) + (mouse.y - hy) * (mouse.y - hy));

    bool hovered = dist <= ctx.handleRadius + 3.0f;
    bool isActive = (ctx.activeId == handleId);

    if (hovered && ctx.activeId == 0) {
        ctx.hoveredId = handleId;
        ctx.hoveredName = id;
    }
    if (hovered && ImGui::IsMouseClicked(0) && ctx.activeId == 0) {
        ctx.activeId = handleId;
        ctx.activeName = id;
    }

    if (isActive && ImGui::IsMouseDown(0)) {
        float ddx = mouse.x - center.x;
        float ddy = mouse.y - center.y;
        float newAngle = std::atan2(ddy, ddx) * 180.0f / 3.14159265f;
        if (ctx.snapGrid > 0)
            newAngle = Snap(newAngle, ctx.snapGrid);
        *angle = newAngle;
        modified = true;
    }

    ImU32 col = isActive ? ctx.colActive : (hovered ? ctx.colHover : ctx.colIdle);
    ctx.drawList->AddCircleFilled({hx, hy}, ctx.handleRadius, col);
    ctx.drawList->AddCircle({hx, hy}, ctx.handleRadius, IM_COL32(0, 0, 0, 180), 0, 1.5f);

    modified |= HandlePoint(id, 0, cx, cy);

    return modified;
}

// drag the square handles to scale each axis. min 0.1.
inline bool Scale(const char* id, float* x, float* y, float* sx, float* sy) {
    auto& ctx = GetContext();
    bool modified = false;

    ImVec2 sp = WorldToScreen(*x, *y);
    float axisLen = 50.0f;

    float sxScreen = axisLen * (*sx);
    float syScreen = axisLen * (*sy);

    ImVec2 xEnd{sp.x + sxScreen, sp.y};
    ImVec2 yEnd{sp.x, sp.y + syScreen};

    ctx.drawList->AddLine(sp, xEnd, ctx.colAxisX, ctx.lineThickness);
    ctx.drawList->AddLine(sp, yEnd, ctx.colAxisY, ctx.lineThickness);

    float boxSz = 5.0f;
    ctx.drawList->AddRectFilled({xEnd.x - boxSz, xEnd.y - boxSz}, {xEnd.x + boxSz, xEnd.y + boxSz}, ctx.colAxisX);
    ctx.drawList->AddRectFilled({yEnd.x - boxSz, yEnd.y - boxSz}, {yEnd.x + boxSz, yEnd.y + boxSz}, ctx.colAxisY);

    ImGuiID xId = HashID(id, 1);
    ImGuiID yId = HashID(id, 2);
    ImVec2 mouse = ImGui::GetMousePos();

    float dxX = mouse.x - xEnd.x, dyX = mouse.y - xEnd.y;
    float distX = std::sqrt(dxX * dxX + dyX * dyX);
    bool hovX = distX <= ctx.handleRadius + 4.0f;
    bool actX = (ctx.activeId == xId);

    if (hovX && ctx.activeId == 0) { ctx.hoveredId = xId; ctx.hoveredName = id; }
    if (hovX && ImGui::IsMouseClicked(0) && ctx.activeId == 0) {
        ctx.activeId = xId; ctx.activeName = id;
        ctx.grabOffset = {mouse.x, 0};
    }
    if (actX && ImGui::IsMouseDown(0)) {
        float delta = (mouse.x - ctx.grabOffset.x) / axisLen;
        *sx += delta;
        if (*sx < 0.1f) *sx = 0.1f;
        ctx.grabOffset = {mouse.x, 0};
        modified = true;
    }

    float dxY = mouse.x - yEnd.x, dyY = mouse.y - yEnd.y;
    float distY = std::sqrt(dxY * dxY + dyY * dyY);
    bool hovY = distY <= ctx.handleRadius + 4.0f;
    bool actY = (ctx.activeId == yId);

    if (hovY && ctx.activeId == 0) { ctx.hoveredId = yId; ctx.hoveredName = id; }
    if (hovY && ImGui::IsMouseClicked(0) && ctx.activeId == 0) {
        ctx.activeId = yId; ctx.activeName = id;
        ctx.grabOffset = {0, mouse.y};
    }
    if (actY && ImGui::IsMouseDown(0)) {
        float delta = (mouse.y - ctx.grabOffset.y) / axisLen;
        *sy += delta;
        if (*sy < 0.1f) *sy = 0.1f;
        ctx.grabOffset = {0, mouse.y};
        modified = true;
    }

    if (actX) ctx.drawList->AddRectFilled({xEnd.x - boxSz, xEnd.y - boxSz}, {xEnd.x + boxSz, xEnd.y + boxSz}, ctx.colActive);
    else if (hovX) ctx.drawList->AddRectFilled({xEnd.x - boxSz, xEnd.y - boxSz}, {xEnd.x + boxSz, xEnd.y + boxSz}, ctx.colHover);

    if (actY) ctx.drawList->AddRectFilled({yEnd.x - boxSz, yEnd.y - boxSz}, {yEnd.x + boxSz, yEnd.y + boxSz}, ctx.colActive);
    else if (hovY) ctx.drawList->AddRectFilled({yEnd.x - boxSz, yEnd.y - boxSz}, {yEnd.x + boxSz, yEnd.y + boxSz}, ctx.colActive);

    modified |= HandlePoint(id, 0, x, y);

    return modified;
}

inline bool Point(const char* id, float* x, float* y) {
    return HandlePoint(id, 0, x, y);
}

inline bool Line(const char* id, float* x1, float* y1, float* x2, float* y2) {
    auto& ctx = GetContext();
    bool modified = false;

    ImVec2 sp1 = WorldToScreen(*x1, *y1);
    ImVec2 sp2 = WorldToScreen(*x2, *y2);

    ctx.drawList->AddLine(sp1, sp2, ctx.colLine, ctx.lineThickness);

    modified |= HandlePoint(id, 0, x1, y1);
    modified |= HandlePoint(id, 1, x2, y2);

    return modified;
}

// flat array of [x,y,x,y,...]. each vertex is draggable.
inline bool Polygon(const char* id, float* points, int count) {
    auto& ctx = GetContext();
    bool modified = false;

    if (count < 2) return false;

    for (int i = 0; i < count; i++) {
        int next = (i + 1) % count;
        ImVec2 a = WorldToScreen(points[i * 2], points[i * 2 + 1]);
        ImVec2 b = WorldToScreen(points[next * 2], points[next * 2 + 1]);
        ctx.drawList->AddLine(a, b, ctx.colLine, ctx.lineThickness);
    }

    for (int i = 0; i < count; i++) {
        modified |= HandlePoint(id, i, &points[i * 2], &points[i * 2 + 1]);
    }

    return modified;
}

inline bool IsHovered() { return GetContext().hoveredId != 0; }
inline bool IsActive() { return GetContext().activeId != 0; }
inline const char* GetActiveId() { return GetContext().activeName; }
inline const char* GetHoveredId() { return GetContext().hoveredName; }

}
