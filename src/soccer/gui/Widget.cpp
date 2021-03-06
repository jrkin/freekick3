#include <stdexcept>

#include "common/Texture.h"

#include "soccer/gui/Widget.h"

namespace Soccer {

using namespace Common;

Widget::Widget(const Rectangle& dim)
	: mRectangle(dim),
	mHidden(false),
	mActive(true)
{
}

bool Widget::clicked(int x, int y)
{
	return mActive && !mHidden && mRectangle.pointWithin(x, y);
}

const Rectangle& Widget::getRectangle() const
{
	return mRectangle;
}

void Widget::setRectangle(const Common::Rectangle& r)
{
	mRectangle = r;
}

bool Widget::visible() const
{
	return !mHidden;
}

bool Widget::hidden() const
{
	return mHidden;
}

void Widget::hide()
{
	mHidden = true;
}

void Widget::show()
{
	mHidden = false;
}

bool Widget::active() const
{
	return mActive;
}

void Widget::activate()
{
	mActive = true;
}

void Widget::deactivate()
{
	mActive = false;
}

}
