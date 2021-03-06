//============================================================================
// Distributed under the MIT License. (See accompanying file LICENSE
// or copy at https://github.com/raphaelmenges/eyeGUI/blob/master/src/LICENSE)
//============================================================================

// Author: Raphael Menges (https://github.com/raphaelmenges)
// Button is an interactive element which can be hit, pushed down or pulled up.
// Hitting triggers a pushing if not down or a pulling if the button is down.

#ifndef BUTTON_H_
#define BUTTON_H_

#include "Elements/InteractiveElements//InteractiveElement.h"
#include "Elements/NotifierTemplate.h"

namespace eyegui
{
	class Button : public InteractiveElement, public NotifierTemplate<ButtonListener>
	{
	public:

		// Constructors
		Button(
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
			std::string iconFilepath,
			bool isSwitch);

		// Destructor
		virtual ~Button() = 0;

		// Press / push button
		virtual void hit(bool immediately = false);
		virtual void down(bool immediately = false);
		virtual void up(bool immediately = false);

		// Is button switch?
		bool isSwitch() const;

		// Is button down?
		bool isDown() const;

	protected:

		// Updating filled by subclasses, returns adaptive scale
		virtual float specialUpdate(float tpf, Input* pInput);

		// Drawing filled by subclasses
		virtual void specialDraw() const;

		// Transformation filled by subclasses
		virtual void specialTransformAndSize();

		// Reset filld by subclasses
		virtual void specialReset();

		// Interaction fill by subclasses
		virtual void specialInteract();

		// Filled by subclass and called by layout after updating and before drawing
		virtual void specialPipeNotification(Notification notification, Layout* pLayout);

	private:

		// Members
		bool mIsDown;
		bool mIsSwitch;
		LerpValue mThreshold; // [0..1]
		LerpValue mPressing; // [0..1]
	};
}

#endif // BUTTON_H_
