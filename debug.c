#pragma once

#include <raylib.h>
#include <box2d/box2d.h>

#include "helpers.c"

void draw_segment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context) {
	DrawLine(p1.x, p1.y, p2.x, p2.y, GetColor(color));
}

void draw_polygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context) {
	for (int i = 0; i < vertexCount - 1; i++) {
		draw_segment(vertices[i], vertices[i + 1], color, context);
	}

	draw_segment(vertices[vertexCount - 1], vertices[0], color, context);
}

void draw_solid_polygon(b2Transform transform, const b2Vec2* vertices, int vertexCount, float radius,
	b2HexColor color, void* context) {
    for (int i = 0; i < vertexCount; ++i) {
        int next_index = (i + 1 == vertexCount) ? 0 : i + 1;
        b2Vec2 p0 = b2TransformPoint(transform, vertices[i]);
        b2Vec2 p1 = b2TransformPoint(transform, vertices[next_index]);
        float x0 = meters_to_pixels(p0.x);
        float y0 = meters_to_pixels(p0.y);
        float x1 = meters_to_pixels(p1.x);
        float y1 = meters_to_pixels(p1.y);

        DrawLine(x0, y0, x1, y1, GetColor(color));
    }
}

void draw_point(b2Vec2 p, float size, b2HexColor color, void* context ) {
	DrawCircle(p.x, p.y, size, GetColor(color));
}
