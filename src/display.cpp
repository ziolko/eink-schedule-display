#include <stdlib.h>
#include <Inkplate.h>

#include "display.hpp"

#include "fonts/roboto_16.h"
#include "fonts/roboto_24.h"
#include "fonts/roboto_36.h"
#include "fonts/roboto_48.h"
#include "fonts/roboto_64.h"

Inkplate display(INKPLATE_1BIT);

int Display::partialUpdateCount = 0;

Display::Display()
{
    hasChanges = false;

    display.begin();
    display.clearDisplay();
    display.setTextColor(BLACK);
}

Display::~Display()
{
    commit();
}

void Display::showErrorScreen(const Error &error)
{
    printOnCenter("Unhandled error: " + error.message, Font::ROBOTO_48);
    commit();
}

void Display::showMessageScreen(const String &message, bool partialUpdate)
{
    printOnCenter(message, Font::ROBOTO_48);
    commit(partialUpdate);
}

void Display::showMessageScreen(const String &message, const String &secondaryMessage, bool partialUpdate)
{
    auto primaryFont = Font::ROBOTO_48;
    auto secondaryFont = Font::ROBOTO_36;
    auto margin = 20;

    setCurrentFont(primaryFont);
    int16_t primaryX, primaryY;
    uint16_t primaryWidth, primaryHeight;
    display.getTextBounds(message, 0, 0, &primaryX, &primaryY, &primaryWidth, &primaryHeight);

    setCurrentFont(secondaryFont);
    int16_t secondaryX, secondaryY;
    uint16_t secondaryWidth, secondaryHeight;
    display.getTextBounds(secondaryMessage, 0, 0, &secondaryX, &secondaryY, &secondaryWidth, &secondaryHeight);

    auto totalHeight = primaryHeight + secondaryHeight + margin;

    setCurrentFont(primaryFont);
    display.setCursor(display.width() / 2 - primaryWidth / 2 - primaryX, display.height() / 2 - totalHeight / 2 - primaryY);
    display.print(message);

    setCurrentFont(secondaryFont);
    display.setCursor(display.width() / 2 - secondaryWidth / 2 - secondaryX, display.height() / 2 - totalHeight / 2 - primaryY + margin + primaryHeight);
    display.print(secondaryMessage);

    hasChanges = true;
    commit(partialUpdate);
}

void Display::setCurrentFont(Font font)
{
    switch (font)
    {
    case Font::ROBOTO_16:
        return display.setFont(&Roboto_16);
    case Font::ROBOTO_24:
        return display.setFont(&Roboto_24);
    case Font::ROBOTO_36:
        return display.setFont(&Roboto_36);
    case Font::ROBOTO_48:
        return display.setFont(&Roboto_48);
    case Font::ROBOTO_64:
        return display.setFont(&Roboto_64);
    }
}

void Display::printOnCenter(const String &text, Font font)
{
    setCurrentFont(font);

    int16_t x1, y1;
    uint16_t width, height;
    display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
    display.setCursor(display.width() / 2 - width / 2 - x1, display.height() / 2 - height / 2 - y1);
    display.print(text);

    hasChanges = true;
}

void Display::print(const String &text, uint x, uint y, Font font, TextAlign textAlign)
{
    setCurrentFont(font);

    int16_t x1, y1;
    uint16_t width, height;
    display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

    if (textAlign == TextAlign::CENTER)
        display.setCursor(display.width() / 2 - width / 2 - x1, y - y1);
    else if (textAlign == TextAlign::LEFT)
        display.setCursor(x - x1, y - y1);
    else if (textAlign == TextAlign::RIGHT)
        display.setCursor(display.width() - x - x1 - width, y - y1);

    display.print(text);

    hasChanges = true;
}

void Display::commit(bool partialUpdate)
{
    if (!hasChanges)
    {
        return;
    }

    if (partialUpdate && Display::partialUpdateCount < 10)
    {
        display.partialUpdate();
        Display::partialUpdateCount += 1;
    }
    else
    {
        display.display();
        Display::partialUpdateCount = 0;
    }

    display.clearDisplay();
    hasChanges = false;
}

void Display::showDeviceScreen(const DeviceState &device)
{
    print(device.getRoomName(), 10, 10, Font::ROBOTO_48);
    print(device.getTime(), 10, 10, Font::ROBOTO_48, TextAlign::RIGHT);

    if (!device.getCurrentMeeting().is_defined && !device.getNextMeeting().is_defined)
    {
        print("Available all day", 0, 280, Font::ROBOTO_64, TextAlign::CENTER);
    }
    else if (!device.getCurrentMeeting().is_defined)
    {
        print("Available", 0, 240, Font::ROBOTO_64, TextAlign::CENTER);
        printNextMeeting(device);
    }
    else
    {
        print("Occupied", 10, 80, Font::ROBOTO_64);
        printCurrentMeeting(device);
        printNextMeeting(device);
    }

    commit();
}
void Display::showConnectionCodeScreen(const DeviceState &device)
{
    print("Connection Code", 0, 190, Font::ROBOTO_48, TextAlign::CENTER);
    print(device.getConnectionCode(), 0, 280, Font::ROBOTO_64, TextAlign::CENTER);
    print("Log in to app.roombelt.com", 0, 370, Font::ROBOTO_36, TextAlign::CENTER);
    print("and use the connection code there.", 0, 410, Font::ROBOTO_36, TextAlign::CENTER);
    return commit();
}

void Display::printCurrentMeeting(const DeviceState &deviceState)
{
    auto current = deviceState.getCurrentMeeting();

    if (current.is_defined)
    {
        print(current.summary, 10, 200, Font::ROBOTO_48);
        print(current.startTime + " - " + current.endTime, 10, 260, Font::ROBOTO_48);
        print("Hosted by " + current.host, 10, 320, Font::ROBOTO_36);
    }
}

void Display::printNextMeeting(const DeviceState &deviceState)
{
    display.drawThickLine(0, 480, display.width(), 480, BLACK, 3);

    auto next = deviceState.getNextMeeting();
    if (next.is_defined)
    {
        print("Next: " + next.summary, 10, 500, Font::ROBOTO_36);
        print(next.startTime + " - " + next.endTime, 10, 550, Font::ROBOTO_36);
    }
    else
    {
        print("No more meetings later today", 10, 500, Font::ROBOTO_36);
    }
}
