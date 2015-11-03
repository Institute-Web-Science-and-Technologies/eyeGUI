//============================================================================
// Distributed under the MIT License. (See accompanying file LICENSE
// or copy at https://github.com/raphaelmenges/eyeGUI/blob/master/src/LICENSE)
//============================================================================

// Author: Raphael Menges (https://github.com/raphaelmenges)

#include "Container.h"

namespace eyegui
{
	Container::Container(
		std::string id,
		std::string styleName,
		Element* pParent,
		Layout const * pLayout,
		Frame* pFrame,
		AssetManager* pAssetManager,
		NotificationQueue* pNotificationQueue,
		float relativeScale,
		float border,
		bool dimmable,
		bool adaptiveScaling,
		float innerBorder) : Block(
			id,
			styleName,
			pParent,
			pLayout,
			pFrame,
			pAssetManager,
			pNotificationQueue,
			relativeScale,
			border,
			dimmable,
			adaptiveScaling,
			innerBorder)
	{
		// Nothing to do
	}

	Container::~Container()
	{
		// Nothing to do
	}

	float Container::specialUpdate(float tpf, Input* pInput)
	{
		float maxAdaptiveScaleOfChildren = 0;

		// Update the elements
		for (std::unique_ptr<Element>& element : mChildren)
		{
			float childAdaptiveScale = element->update(tpf, mAlpha, pInput, mDimming.getValue());
			maxAdaptiveScaleOfChildren = std::max(maxAdaptiveScaleOfChildren, childAdaptiveScale);
		}

		// Super call after children (may consume input first)
		float adaptiveScale = Block::specialUpdate(tpf, pInput);

		// Return adaptive scale
		return std::max(adaptiveScale, maxAdaptiveScaleOfChildren);
	}

	void Container::specialDraw() const
	{
		Block::specialDraw();

		// Draw the elements
		for (const std::unique_ptr<Element>& element : mChildren)
		{
			element->draw();
		}
	}
}
