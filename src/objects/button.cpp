//Default Button Object

#include <bits/stdc++.h>
#include "raylib.h"
using namespace std;

struct Button {
    Rectangle rect;
    Color backgroundColor;
    Color textColor;
    string text;
    Font font;
    float fontSize;
    float spacing;
    enum Align { LEFT, CENTER, RIGHT } align;
    float padding;
    Color borderColor;
    int borderWeight;
    float borderRadius;
    Button(float positionX, float positionY,
           Color backgroundColor, Color textColor,
           float fontSize, float width, float height,
           const string &text, Font font,
           const string &textAlignment = "center",
           float padding = 4.0f,
           Color borderColor = BLACK, int borderWeight = 0,
           float borderRadius = 0.0f,
           float spacing = 0.0f) {
        rect = {positionX, positionY, width, height};
        this->backgroundColor = backgroundColor;
        this->textColor = textColor;
        this->text = text;
        this->font = font;
        this->fontSize = fontSize;
        this->spacing = spacing;
        this->padding = padding;
        this->borderColor = borderColor;
        this->borderWeight = borderWeight;
        this->borderRadius = borderRadius;

        if (textAlignment == "left") align = LEFT;
        else if (textAlignment == "right") align = RIGHT;
        else align = CENTER;
    }

    void Draw() {
        if (borderRadius > 0.0f) {
            DrawRectangleRounded(rect, borderRadius, 6, backgroundColor);
            if (borderWeight > 0) DrawRectangleRoundedLines(rect, borderRadius, 6, borderColor);
        } else {
            DrawRectangleRec(rect, backgroundColor);
            if (borderWeight > 0) {
                for (int i = 0; i < borderWeight; ++i) {
                    DrawRectangleLines((int)rect.x - i, (int)rect.y - i,
                                       (int)rect.width + 2*i, (int)rect.height + 2*i,
                                       borderColor);
                }
            }
        }

        Rectangle inner = {rect.x + padding, rect.y + padding, rect.width - 2*padding, rect.height - 2*padding};

        Vector2 textSize = MeasureTextEx(font, text.c_str(), fontSize, spacing);
        Vector2 pos;
        pos.y = inner.y + (inner.height - textSize.y) / 2.0f;
        if (align == LEFT) pos.x = inner.x;
        else if (align == RIGHT) pos.x = inner.x + inner.width - textSize.x;
        else pos.x = inner.x + (inner.width - textSize.x) / 2.0f;

        DrawTextEx(font, text.c_str(), pos, fontSize, spacing, textColor);
    }
};