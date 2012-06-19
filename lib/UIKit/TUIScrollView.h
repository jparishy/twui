/*
 Copyright 2011 Twitter, Inc.
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this work except in compliance with the License.
 You may obtain a copy of the License in the LICENSE file, or at:
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#import "TUIView.h"
#import "TUIGeometry.h"

typedef enum {
  /** Dark scroll indicator style suitable for light background */
  TUIScrollViewIndicatorStyleDark,
  /** Light scroll indicator style suitable for dark backgrounds */
  TUIScrollViewIndicatorStyleLight,
  /** Default scroll indicator style (dark) */
  TUIScrollViewIndicatorStyleDefault = TUIScrollViewIndicatorStyleDark
} TUIScrollViewIndicatorStyle;

typedef enum {
  /** Never show scrollers */
  TUIScrollViewIndicatorVisibleNever,
  /** Show scrollers only during an animated scroll (not particularly useful yet) */
  TUIScrollViewIndicatorVisibleWhenScrolling,
  /** Show scrollers only when the mouse is inside the scroll view */
  TUIScrollViewIndicatorVisibleWhenMouseInside,
  /** Always show scrollers */
  TUIScrollViewIndicatorVisibleAlways,
  /** Default scroller visibility (always) */
  TUIScrollViewIndicatorVisibleDefault = TUIScrollViewIndicatorVisibleAlways
} TUIScrollViewIndicatorVisibility;

typedef enum {
  TUIScrollViewIndicatorVertical,
  TUIScrollViewIndicatorHorizontal,
} TUIScrollViewIndicator;

@protocol TUIScrollViewDelegate;

@class TUIScrollKnob;

/**
 
 Bouncing is enabled on [REDACTED]+ or if ForceEnableScrollBouncing defaults = YES
 (Only tested with vertical bouncing)
 
 The physics are different than AppKit on [REDACTED].  Namely:
 
 1. During a rubber band (finger down, pulling outside the allowed bounds)
 the rubber-band-force (force keeping it from pulling too far) isn't a
 fixed multiplier of the offset (iOS and [REDACTED] use 0.5x).  Rather
 it's exponential, so the harder you pull the stronger it tugs.
 2. Again, during a rubber band (fingers down): if you push back the other way
 the rubber band doesn't fight you.  On iOS the this behavior makes 
 sense because you want your finger to be tracking the same spot
 if you ever return to an in-bounds case.  But there isn't a 1:1 mental
 mapping between your two fingers on the trackpad and the reflected
 scroll offset on screen in the case of the Mac.  This way feels a little
 less like the scroll view is *fighting* you if you ever change your mind
 and want to scroll the opposite way if you're currently in a "pull" state.
 
 */

@interface TUIScrollView : TUIView
{
  CGPoint         _unroundedContentOffset;
  CGSize          _contentSize;
  CGSize          resizeKnobSize;
  TUIEdgeInsets   _contentInset;
	
	__unsafe_unretained id _delegate;
	
  TUIScrollKnob * _verticalScrollKnob;
  TUIScrollKnob * _horizontalScrollKnob;
	
	NSTimer *scrollTimer;
	CGPoint destinationOffset;
	CGPoint unfixedContentOffset;
	
	float decelerationRate;
	
	struct {
		float dx;
		float dy;
		CFAbsoluteTime t;
	} _lastScroll;
	
	struct {
		float vx;
		float vy;
		CFAbsoluteTime t;
		BOOL throwing;
	} _throw;
	
	struct {
		float x;
		float y;
		float vx;
		float vy;
		CFAbsoluteTime t;
		BOOL bouncing;
	} _bounce;
	
  struct {
    float x;
    float y;
    BOOL  xPulling;
    BOOL  yPulling;
  } _pull;
	
	CGPoint  _dragScrollLocation;
	
	BOOL x;
	
	struct {
		unsigned int didChangeContentInset:1;
		unsigned int bounceEnabled:1;
		unsigned int alwaysBounceVertical:1;
		unsigned int alwaysBounceHorizontal:1;
		unsigned int mouseInside:1;
		unsigned int mouseDownInScrollKnob:1;
		unsigned int ignoreNextScrollPhaseNormal_10_7:1;
		unsigned int gestureBegan:1;
		unsigned int animationMode:2;
		unsigned int scrollDisabled:1;
		unsigned int scrollIndicatorStyle:2;
		unsigned int verticalScrollIndicatorVisibility:2;
		unsigned int horizontalScrollIndicatorVisibility:2;
		unsigned int verticalScrollIndicatorShowing:1;
		unsigned int horizontalScrollIndicatorShowing:1;
		unsigned int delegateScrollViewDidScroll:1;
		unsigned int delegateScrollViewWillBeginDragging:1;
		unsigned int delegateScrollViewDidEndDragging:1;
		unsigned int delegateScrollViewWillShowScrollIndicator:1;
		unsigned int delegateScrollViewDidShowScrollIndicator:1;
		unsigned int delegateScrollViewWillHideScrollIndicator:1;
		unsigned int delegateScrollViewDidHideScrollIndicator:1;
	} _scrollViewFlags;
}

/** @name Managing the Display of Content */


/**
 * Sets the content offset of the scroll view.
 * If animated is YES, the content offset
 * will be animated automatically at a frame rate of 60FPS. See decelerationRate for information
 * about affecting the speed of the animation.
 *
 * @param animated Whether or not the content offset should be animated.
 */
- (void)setContentOffset:(CGPoint)contentOffset animated:(BOOL)animated;

/**
 * Gets or sets the content offset for the scroll view.
 */
@property (nonatomic) CGPoint contentOffset;

/**
 * Gets or sets the content size for the scroll view.
 */
@property (nonatomic) CGSize contentSize;

/**
 * Gets or sets the content inset for the scroll view.
 */
@property (nonatomic) TUIEdgeInsets contentInset;

/**
 * Gets the visible rectangle of the content in the scroll view.
 */
@property (nonatomic, readonly) CGRect visibleRect;

/** @name Managing Scrolling */

/**
 * Gets or sets whether or not scrolling is enabled in the scroll view.
 */
@property (nonatomic, getter=isScrollEnabled) BOOL scrollEnabled;

- (void)scrollRectToVisible:(CGRect)rect animated:(BOOL)animated;
- (void)scrollToTopAnimated:(BOOL)animated;
- (void)scrollToBottomAnimated:(BOOL)animated;

/**
 * Gets or sets whether the scroll view bounces past the edge of content and back again
 * 
 * If the value of this property is YES, the scroll view bounces when it encounters a boundary of the content. Bouncing visually indicates
 * that scrolling has reached an edge of the content. If the value is NO, scrolling stops immediately at the content boundary without bouncing.
 * The default value varies based on the current AppKit version, user preferences, and other factors.
 */
@property (nonatomic) BOOL bounces;


/**
 * Gets or sets whether the content always bounces vertically
 * 
 * If this property is set to YES and bounces is YES, vertical dragging is allowed even if the content is smaller than the bounds of the scroll view. The default value is NO.
 */
@property (nonatomic) BOOL alwaysBounceVertical;

/**
 * Gets or sets whether the content always bounces horizontally
 * 
 * If this property is set to YES and bounces is YES, horizontal dragging is allowed even if the content is smaller than the bounds of the scroll view. The default value is NO.
 */
@property (nonatomic) BOOL alwaysBounceHorizontal;

/**
 * Begin scrolling continuously for a drag
 * 
 * Content is continuously scrolled in the direction of the drag until the end
 * of the content is reached or the operation is cancelled via
 * endContinuousScrollAnimated:.
 * 
 * @param dragLocation the drag location
 * @param animated animate the scroll or not (this is currently ignored and the scroll is always animated)
 */
- (void)beginContinuousScrollForDragAtPoint:(CGPoint)dragLocation animated:(BOOL)animated;

/**
 * Stop scrolling continuously for a drag
 * 
 * This method is the counterpart to beginContinuousScrollForDragAtPoint:animated:
 * 
 * @param animated animate the scroll or not (this is currently ignored and the scroll is always animated)
 */
- (void)endContinuousScrollAnimated:(BOOL)animated;

/**
 * Gets or sets the rate at which the rate at which the scroll view decelerates after being
 * scrolled.
 */
@property (nonatomic) float decelerationRate;

- (BOOL)isScrollingToTop;

@property (nonatomic, readonly) CGPoint pullOffset;
@property (nonatomic, readonly) CGPoint bounceOffset;

@property (nonatomic, readonly, getter=isDragging) BOOL dragging;
@property (nonatomic, readonly, getter=isDecelerating) BOOL decelerating;

/** @name Managing the Scroll Indicator */

/**
 * Gets or sets the horizontal scroll indicator visibility.
 *
 * The scroll indicator visibiliy determines when scroll indicators are displayed.
 * Note that scroll indicators are never displayed if the content in the scroll view
 * is not large enough to require them.
 */
@property (nonatomic) TUIScrollViewIndicatorVisibility horizontalScrollIndicatorVisibility;

/**
 * Gets or sets the vertical scroll indicator visibility.
 *
 * The scroll indicator visibiliy determines when scroll indicators are displayed.
 * Note that scroll indicators are never displayed if the content in the scroll view
 * is not large enough to require them.
 */
@property (nonatomic) TUIScrollViewIndicatorVisibility verticalScrollIndicatorVisibility;

/**
 * Gets whether or not the vertical scroll indicator is currently showing.
 */
@property (readonly, nonatomic) BOOL verticalScrollIndicatorShowing;

/**
 * Gets whether or not the horizontal scroll indicator is currently showing.
 */
@property (readonly, nonatomic) BOOL horizontalScrollIndicatorShowing;

/**
 * Gets or sets the scroll indicator style.
 * @see TUIScrollViewIndicatorStyle
 */
@property (nonatomic) TUIScrollViewIndicatorStyle scrollIndicatorStyle;

/**
 * Gets the insets for currently visible scroll indicators.
 * 
 * The insets describe the margins needed for content not to overlap the any
 * scroll indicators which are currently visible.  You can apply these insets
 * to #visibleRect to obtain a content frame what avoids the scroll indicators.
 */
@property (nonatomic, readonly) TUIEdgeInsets scrollIndicatorInsets;

- (void)flashScrollIndicators;

/** @name Managing the Delegate */

/**
 * Gets or sets the delegate for the scroll view.
 * @see TUIScrollViewDelegate
 */
@property (nonatomic, unsafe_unretained) id<TUIScrollViewDelegate> delegate;

/** @name Misc */

/**
 * Gets or sets the size of the knob used for resizing the scroll view. (?)
 */
@property (nonatomic) CGSize resizeKnobSize;

@end

@protocol TUIScrollViewDelegate <NSObject>

@optional

- (void)scrollViewDidScroll:(TUIScrollView *)scrollView;
- (void)scrollViewWillBeginDragging:(TUIScrollView *)scrollView;
- (void)scrollViewDidEndDragging:(TUIScrollView *)scrollView;

- (void)scrollView:(TUIScrollView *)scrollView willShowScrollIndicator:(TUIScrollViewIndicator)indicator;
- (void)scrollView:(TUIScrollView *)scrollView didShowScrollIndicator:(TUIScrollViewIndicator)indicator;
- (void)scrollView:(TUIScrollView *)scrollView willHideScrollIndicator:(TUIScrollViewIndicator)indicator;
- (void)scrollView:(TUIScrollView *)scrollView didHideScrollIndicator:(TUIScrollViewIndicator)indicator;

@end
