// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_TAB_CONTENTS_TAB_CONTENTS_VIEW_VIEWS_H_
#define CHROME_BROWSER_UI_VIEWS_TAB_CONTENTS_TAB_CONTENTS_VIEW_VIEWS_H_
#pragma once

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/timer.h"
#include "chrome/browser/ui/views/tab_contents/native_tab_contents_view_delegate.h"
#include "content/browser/tab_contents/tab_contents_view_helper.h"
#include "content/public/browser/web_contents_view.h"
#include "ui/views/widget/widget.h"

class NativeTabContentsView;
class RenderViewContextMenuViews;
class SkBitmap;
struct WebDropData;
namespace gfx {
class Point;
class Size;
}
namespace views {
class Widget;
}
namespace content {
class WebContentsViewDelegate;
class WebDragDestDelegate;
}

// Views-specific implementation of the WebContentsView.
class TabContentsViewViews : public views::Widget,
                             public content::WebContentsView,
                             public internal::NativeTabContentsViewDelegate {
 public:
  // The corresponding WebContents is passed in the constructor, and manages our
  // lifetime. This doesn't need to be the case, but is this way currently
  // because that's what was easiest when they were split.
  explicit TabContentsViewViews(content::WebContents* web_contents,
                                content::WebContentsViewDelegate* delegate);
  virtual ~TabContentsViewViews();

  NativeTabContentsView* native_tab_contents_view() const {
    return native_tab_contents_view_;
  }

  // Overridden from WebContentsView:
  virtual void CreateView(const gfx::Size& initial_size) OVERRIDE;
  virtual content::RenderWidgetHostView* CreateViewForWidget(
      content::RenderWidgetHost* render_widget_host) OVERRIDE;
  virtual gfx::NativeView GetNativeView() const OVERRIDE;
  virtual gfx::NativeView GetContentNativeView() const OVERRIDE;
  virtual gfx::NativeWindow GetTopLevelNativeWindow() const OVERRIDE;
  virtual void GetContainerBounds(gfx::Rect* out) const OVERRIDE;
  virtual void SetPageTitle(const string16& title) OVERRIDE;
  virtual void OnTabCrashed(base::TerminationStatus status,
                            int error_code) OVERRIDE;
  virtual void SizeContents(const gfx::Size& size) OVERRIDE;
  virtual void RenderViewCreated(content::RenderViewHost* host) OVERRIDE;
  virtual void Focus() OVERRIDE;
  virtual void SetInitialFocus() OVERRIDE;
  virtual void StoreFocus() OVERRIDE;
  virtual void RestoreFocus() OVERRIDE;
  virtual bool IsDoingDrag() const OVERRIDE;
  virtual void CancelDragAndCloseTab() OVERRIDE;
  virtual bool IsEventTracking() const OVERRIDE;
  virtual void CloseTabAfterEventTracking() OVERRIDE;
  virtual void GetViewBounds(gfx::Rect* out) const OVERRIDE;

  // Implementation of RenderViewHostDelegate::View.
  virtual void CreateNewWindow(
      int route_id,
      const ViewHostMsg_CreateWindow_Params& params) OVERRIDE;
  virtual void CreateNewWidget(int route_id,
                               WebKit::WebPopupType popup_type) OVERRIDE;
  virtual void CreateNewFullscreenWidget(int route_id) OVERRIDE;
  virtual void ShowCreatedWindow(int route_id,
                                 WindowOpenDisposition disposition,
                                 const gfx::Rect& initial_pos,
                                 bool user_gesture) OVERRIDE;
  virtual void ShowCreatedWidget(int route_id,
                                 const gfx::Rect& initial_pos) OVERRIDE;
  virtual void ShowCreatedFullscreenWidget(int route_id) OVERRIDE;
  virtual void ShowContextMenu(
      const content::ContextMenuParams& params) OVERRIDE;
  virtual void ShowPopupMenu(const gfx::Rect& bounds,
                             int item_height,
                             double item_font_size,
                             int selected_item,
                             const std::vector<WebMenuItem>& items,
                             bool right_aligned) OVERRIDE;
  virtual void StartDragging(const WebDropData& drop_data,
                             WebKit::WebDragOperationsMask operations,
                             const SkBitmap& image,
                             const gfx::Point& image_offset) OVERRIDE;
  virtual void UpdateDragCursor(WebKit::WebDragOperation operation) OVERRIDE;
  virtual void GotFocus() OVERRIDE;
  virtual void TakeFocus(bool reverse) OVERRIDE;

 private:
  // Overridden from internal::NativeTabContentsViewDelegate:
  virtual content::WebContents* GetWebContents() OVERRIDE;
  virtual void OnNativeTabContentsViewShown() OVERRIDE;
  virtual void OnNativeTabContentsViewHidden() OVERRIDE;
  virtual void OnNativeTabContentsViewSized(const gfx::Size& size) OVERRIDE;
  virtual void OnNativeTabContentsViewWheelZoom(bool zoom_in) OVERRIDE;
  virtual void OnNativeTabContentsViewMouseDown() OVERRIDE;
  virtual void OnNativeTabContentsViewMouseMove(bool motion) OVERRIDE;
  virtual void OnNativeTabContentsViewDraggingEnded() OVERRIDE;
  virtual views::internal::NativeWidgetDelegate*
      AsNativeWidgetDelegate() OVERRIDE;
  virtual content::WebDragDestDelegate* GetDragDestDelegate() OVERRIDE;

  // Overridden from views::Widget:
  virtual void OnNativeWidgetVisibilityChanged(bool visible) OVERRIDE;
  virtual void OnNativeWidgetSizeChanged(const gfx::Size& new_size) OVERRIDE;

  // A helper method for closing the tab.
  void CloseTab();

  views::Widget* GetSadTab() const;

  // ---------------------------------------------------------------------------

  // The WebContents whose contents we display.
  content::WebContents* web_contents_;

  // Common implementations of some WebContentsView methods.
  TabContentsViewHelper tab_contents_view_helper_;

  NativeTabContentsView* native_tab_contents_view_;

  // The id used in the ViewStorage to store the last focused view.
  int last_focused_view_storage_id_;

  // The context menu. Callbacks are asynchronous so we need to keep it around.
  scoped_ptr<RenderViewContextMenuViews> context_menu_;

  // Set to true if we want to close the tab after the system drag operation
  // has finished.
  bool close_tab_after_drag_ends_;

  // Used to close the tab after the stack has unwound.
  base::OneShotTimer<TabContentsViewViews> close_tab_timer_;

  // Chrome specific functionality (to make it easier for this class to
  // eventually move to content).
  scoped_ptr<content::WebContentsViewDelegate> delegate_;

  DISALLOW_COPY_AND_ASSIGN(TabContentsViewViews);
};

#endif  // CHROME_BROWSER_UI_VIEWS_TAB_CONTENTS_TAB_CONTENTS_VIEW_VIEWS_H_
