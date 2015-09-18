//============================================================================
// Distributed under the MIT License. (See accompanying file LICENSE 
// or copy at https://github.com/raphaelmenges/eyeGUI/blob/master/src/LICENSE)
//============================================================================

// Author: Raphael Menges (https://github.com/raphaelmenges)

#include "Element.h"

#include "Layout.h"
#include "Helper.h"
#include "External/GLM/glm/gtc/matrix_transform.hpp"

#include <algorithm>

namespace eyegui
{
	Element::Element(
		std::string id,
		std::string styleName,
		Element* pParent,
		Layout* pLayout,
		AssetManager* pAssetManager,
		float relativeScale,
		float border)
	{
		// Initialize members
		mType = Type::ELEMENT;
		mId = id;
		mStyleName = styleName;
		mpParent = pParent;
		mpLayout = pLayout;
		mpAssetManager = pAssetManager;
		mX, mY, mWidth, mHeight = 0;
		mRelativeScale = relativeScale;
		mBorder = border;
		mActive = true;
		mActivity = 1;
		mAlpha = 1;
		mBorderAspectRatio = 1;

		// Fetch style from layout
		mpStyle = mpLayout->getStyleFromStylesheet(mStyleName);
	}

	Element::~Element()
	{
		// Nothing to do so far
	}

	Element::Type Element::getType() const
	{
		return mType;
	}

	std::string Element::getId() const
	{
		return mId;
	}

	Element* Element::getParent() const
	{
		return mpParent;
	}

	bool Element::isActive() const
	{
		return mActive;
	}

	Element::Orientation Element::getOrientation() const
	{
		return mOrientation;
	}

	Style const * Element::getStyle() const
	{
		return mpStyle;
	}

	std::string Element::getStyleName() const
	{
		return mStyleName;
	}

	void Element::setAlpha(float alpha)
	{
		mAlpha = alpha;
	}

	float Element::getAlpha() const
	{
		return mAlpha;
	}

	void Element::setActivity(bool active, bool setImmediately)
	{
		if (active == mActive)
		{
			// Nothing to do
			return;
		}
		else
		{
			if (active)
			{
				if (mpParent->isActive())
				{
					mActive = true;

					// Do it immediatelly, maybe
					if (setImmediately)
					{
						mActivity = 1;
					}

					// Do it for all children
					for (const std::unique_ptr<Element>& element : mChildren)
					{
						element.get()->setActivity(true, setImmediately);
					}
				}
			}
			else
			{
				mActive = false;

				// Do it immediatelly, maybe
				if (setImmediately)
				{
					mActivity = 0;
				}

				// Do it for all children
				for (const std::unique_ptr<Element>& element : mChildren)
				{
					element.get()->setActivity(false, setImmediately);
				}
			}
		}
	}

	Layout* Element::getLayout() const
	{
		return mpLayout;
	}

	AssetManager* Element::getAssetManager() const
	{
		return mpAssetManager;
	}

	float Element::getBorder() const
	{
		return mBorder;
	}

	std::set<Element*> Element::getAllChildren()
	{
		std::set<Element*> elements;

		// Go over children and collect pointers
		for (std::unique_ptr<Element>& rupChild : mChildren)
		{
			std::set<Element*> childrensElements = rupChild->getAllChildren();
			elements.insert(childrensElements.begin(), childrensElements.end());
			elements.insert(rupChild.get());
		}

		return elements;
	}

	std::set<std::string> Element::getAllChildrensIds() const
	{
		std::set<std::string> ids;

		// Go over children and collect ids
		for (const std::unique_ptr<Element>& rupChild : mChildren)
		{
			// Insert id of children of child
			std::set<std::string> childrensIds = rupChild->getAllChildrensIds();
			ids.insert(childrensIds.begin(), childrensIds.end());

			// Only insert id of child if there is one
			std::string id = rupChild->getId();
			if (id != "")
			{
				ids.insert(rupChild->getId());
			}
		}

		return ids;
	}

	void Element::transformAndSize(int x, int y, int width, int height)
	{
		// Use border
		int usedBorder;
		if (width > height)
		{
			usedBorder = (int)((float)height * mBorder);
		}
		else
		{
			usedBorder = (int)((float)width * mBorder);
		}

		// Keep aspect ratio in mind (for example: keep aspect ratio of picture)
		if (mBorderAspectRatio > 1)
		{
			// Wider
			x += (int)((usedBorder / 2) * mBorderAspectRatio);
			width -= (int)((usedBorder)* mBorderAspectRatio);

			y += usedBorder / 2;
			height -= usedBorder;
		}
		else
		{
			// Higher
			x += usedBorder / 2;
			width -= usedBorder;

			y += (int)((usedBorder / 2) * mBorderAspectRatio);
			height -= (int)(usedBorder * mBorderAspectRatio);
		}

		// Save values to members
		mX = x;
		mY = y;
		mWidth = width;
		mHeight = height;

		// Set orientation
		if (mWidth > mHeight)
		{
			mOrientation = Element::Orientation::HORIZONTAL;
		}
		else
		{
			mOrientation = Element::Orientation::VERTICAL;
		}

		// Call method implemented by subclasses
		specialTransformAndSize();

		// After calculation transformation, recalculate the matrix for rendering
		mDrawMatrix = calculateDrawMatrix(mX, mY, mWidth, mHeight);
	}


	int Element::getX() const
	{
		return mX;
	}
	int Element::getY() const
	{
		return mY;
	}

	int Element::getWidth() const
	{
		return mWidth;
	}

	int Element::getHeight() const
	{
		return mHeight;
	}

	float Element::getRelativeScale() const
	{
		return mRelativeScale;
	}

	void Element::update(float tpf, float alpha, Input* pInput)
	{
		// Activity animation
		if (mActive)
		{
			mActivity += tpf;
		}
		else
		{
			mActivity -= tpf;
		}
		mActivity = clamp(mActivity, 0, 1);

		// Save current alpha (already animated by layout or other element)
		mAlpha = alpha;

		// Use activity and alpha to check whether input is necessary
		if (mAlpha < 1 || mActivity < 1)
		{
			pInput = NULL;
		}

		// Update replaced element if there is some
		if (mupReplacedElement.get() != NULL)
		{
			float replacedAlpha = mAlpha * (mupReplacedElement->getAlpha()
				- (tpf / mpLayout->getConfig()->animationDuration));
			replacedAlpha = clamp(replacedAlpha, 0, 1);
			mupReplacedElement->update(tpf, replacedAlpha, NULL);

			// Check, whether replacement is still visible
			if (replacedAlpha <= 0)
			{
				// Give it to layout for destruction at end of frame
				mpLayout->commitDyingReplacedElement(std::move(mupReplacedElement));
			}
		}

		// Call specialized update of subclasses
		specialUpdate(tpf, pInput);
	}

	void Element::draw() const
	{
		// Only draw if visible
		if (mAlpha > 0)
		{
			// Draw the element
			specialDraw();

			// Draw fading replaced elements if available (always mutliplied with own alpha)
			if (mupReplacedElement.get() != NULL)
			{
				mupReplacedElement->draw();
			}
		}
	}

	void Element::reset()
	{
		mActive = true;
		mActivity = 1;

		// Do own reset implemented by subclass
		specialReset();

		// Go over chilren and reset
		for (std::unique_ptr<Element>& element : mChildren)
		{
			element->reset();
		}
	}

	void Element::evaluateSize(
		int availableWidth,
		int availableHeight,
		int& rWidth,
		int& rHeight) const
	{
		rWidth = availableWidth;
		rHeight = availableHeight;
	}

	InteractiveElement* Element::nextInteractiveElement()
	{
		if (getParent() != NULL)
		{
			// Ok, go one up and look. Only interactive elements should be called.
			return getParent()->internalNextInteractiveElement(this);
		}
		else
		{
			// Top layer, call yourself
			return internalNextInteractiveElement(NULL);
		}
	}

	InteractiveElement* Element::internalNextInteractiveElement(Element const * pChildCaller)
	{
		// This is no interactive element ...
		if (getParent() != NULL)
		{
			// ... so go one up to parent a hope the best.
			return getParent()->internalNextInteractiveElement(this);
		}
		else
		{
			// ... so it is root and no interactive element can be selected
			return NULL;
		}
	}

	std::unique_ptr<Element> Element::replaceAttachedElement(Element* pTarget,
		std::unique_ptr<Element> upReplacement)
	{
		// Search in children for element to replace
		int index = -1;
		int i = 0;
		for (std::unique_ptr<Element>& rupElement : mChildren)
		{
			if (rupElement.get() == pTarget)
			{
				index = i;
				break;
			}
			i++;
		}

		// Child found, so replace it
		if (index > -1)
		{
			// Replace it
			std::unique_ptr<Element> upTarget = std::move(mChildren[i]);
			mChildren[i] = std::move(upReplacement);
			return std::move(upTarget);
		}

		// Fallback if not found
		return NULL;
	}

	void Element::commitReplacedElement(std::unique_ptr<Element> upElement, bool doFading)
	{
		if (doFading)
		{
			// Fading is wished, so remember the replaced element
			mupReplacedElement = std::move(upElement);
		}
		else
		{
			// No fading, so let it die instantly after this frame
			mpLayout->commitDyingReplacedElement(std::move(upElement));
		}
	}

	glm::mat4 Element::calculateDrawMatrix(int x, int y, int width, int height) const
	{
		// Get values from layout
		float windowWidth = (float)(mpLayout->getLayoutWidth());
		float windowHeight = (float)(mpLayout->getLayoutHeight());

		// Create identity
		glm::mat4 matrix = glm::mat4(1.0f);

		// Width and height from zero to one
		float glWidth = width / windowWidth;
		float glHeight = height / windowHeight;

		// Moving
		matrix = glm::translate(
			matrix,
			glm::vec3(x / windowWidth,
				(1.0f - (y / windowHeight)) - glHeight,
				0));

		// Scaling
		matrix = glm::scale(matrix, glm::vec3(glWidth, glHeight, 1));

		// Projection
		matrix = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f) * matrix;

		return matrix;
	}
}