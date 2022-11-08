// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2012 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt.android;

import android.app.Activity;
import android.content.Context;
import android.os.Build;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowInsets;
import android.view.WindowManager;

public class QtLayout extends ViewGroup
{
    private Runnable m_startApplicationRunnable;

    private int m_activityDisplayRotation = -1;
    private int m_ownDisplayRotation = -1;
    private int m_nativeOrientation = -1;

    public void setActivityDisplayRotation(int rotation)
    {
        m_activityDisplayRotation = rotation;
    }

    public void setNativeOrientation(int orientation)
    {
        m_nativeOrientation = orientation;
    }

    public int displayRotation()
    {
        return m_ownDisplayRotation;
    }

    public QtLayout(Context context, Runnable startRunnable)
    {
        super(context);
        m_startApplicationRunnable = startRunnable;
    }

    public QtLayout(Context context, AttributeSet attrs)
    {
        super(context, attrs);
    }

    public QtLayout(Context context, AttributeSet attrs, int defStyle)
    {
        super(context, attrs, defStyle);
    }

    @Override
    protected void onSizeChanged (int w, int h, int oldw, int oldh)
    {
        WindowInsets insets = getRootWindowInsets();

        DisplayMetrics realMetrics = new DisplayMetrics();
        Display display = (Build.VERSION.SDK_INT < Build.VERSION_CODES.R)
                ? ((Activity)getContext()).getWindowManager().getDefaultDisplay()
                : ((Activity)getContext()).getDisplay();
        display.getRealMetrics(realMetrics);

        if ((realMetrics.widthPixels > realMetrics.heightPixels) != (w > h)) {
            // This is an intermediate state during display rotation.
            // The new size is still reported for old orientation, while
            // realMetrics contain sizes for new orientation. Setting
            // such parameters will produce inconsistent results, so
            // we just skip them.
            // We will have another onSizeChanged() with normal values
            // a bit later.
            return;
        }

        boolean isFullScreenView = h == realMetrics.heightPixels;
        // The code uses insets for fullscreen mode only. However in practice
        // the insets can be reported incorrectly. Both on Android 6 and Android 11
        // a non-zero bottom inset is reported even when the
        // WindowManager.LayoutParams.FLAG_FULLSCREEN flag is set.
        // To avoid that, add an extra check for the fullscreen mode.
        // The insets-related logic is not removed for the case when
        // isFullScreenView == true, but hasFlagFullscreen == false, although
        // I can't get such case in my tests.
        final int windowFlags = ((Activity)getContext()).getWindow().getAttributes().flags;
        final boolean hasFlagFullscreen =
                (windowFlags & WindowManager.LayoutParams.FLAG_FULLSCREEN) != 0;
        int insetLeft =
                (isFullScreenView && !hasFlagFullscreen) ? insets.getSystemWindowInsetLeft() : 0;
        int insetTop =
                (isFullScreenView && !hasFlagFullscreen) ? insets.getSystemWindowInsetTop() : 0;
        int insetRight =
                (isFullScreenView && !hasFlagFullscreen) ? insets.getSystemWindowInsetRight() : 0;
        int insetBottom =
                (isFullScreenView && !hasFlagFullscreen) ? insets.getSystemWindowInsetBottom() : 0;

        int usableAreaWidth = w - insetLeft - insetRight;
        int usableAreaHeight = h - insetTop - insetBottom;

        QtNative.setApplicationDisplayMetrics(
                realMetrics.widthPixels, realMetrics.heightPixels, insetLeft, insetTop,
                usableAreaWidth, usableAreaHeight, realMetrics.xdpi, realMetrics.ydpi,
                realMetrics.scaledDensity, realMetrics.density, display.getRefreshRate());

        int newRotation = display.getRotation();
        if (m_ownDisplayRotation != m_activityDisplayRotation
            && newRotation == m_activityDisplayRotation) {
            // If the saved rotation value does not match the one from the
            // activity, it means that we got orientation change before size
            // change, and the value was cached. So we need to notify about
            // orientation change now.
            QtNative.handleOrientationChanged(newRotation, m_nativeOrientation);
        }
        m_ownDisplayRotation = newRotation;

        if (m_startApplicationRunnable != null) {
            m_startApplicationRunnable.run();
            m_startApplicationRunnable = null;
        }
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec)
    {
        int count = getChildCount();

        int maxHeight = 0;
        int maxWidth = 0;

        // Find out how big everyone wants to be
        measureChildren(widthMeasureSpec, heightMeasureSpec);

        // Find rightmost and bottom-most child
        for (int i = 0; i < count; i++) {
            View child = getChildAt(i);
            if (child.getVisibility() != GONE) {
                int childRight;
                int childBottom;

                QtLayout.LayoutParams lp
                        = (QtLayout.LayoutParams) child.getLayoutParams();

                childRight = lp.x + child.getMeasuredWidth();
                childBottom = lp.y + child.getMeasuredHeight();

                maxWidth = Math.max(maxWidth, childRight);
                maxHeight = Math.max(maxHeight, childBottom);
            }
        }

        // Check against minimum height and width
        maxHeight = Math.max(maxHeight, getSuggestedMinimumHeight());
        maxWidth = Math.max(maxWidth, getSuggestedMinimumWidth());

        setMeasuredDimension(resolveSize(maxWidth, widthMeasureSpec),
                resolveSize(maxHeight, heightMeasureSpec));
    }

    /**
    * Returns a set of layout parameters with a width of
    * {@link android.view.ViewGroup.LayoutParams#WRAP_CONTENT},
    * a height of {@link android.view.ViewGroup.LayoutParams#WRAP_CONTENT}
    * and with the coordinates (0, 0).
    */
    @Override
    protected ViewGroup.LayoutParams generateDefaultLayoutParams()
    {
        return new LayoutParams(android.view.ViewGroup.LayoutParams.WRAP_CONTENT,
                                android.view.ViewGroup.LayoutParams.WRAP_CONTENT,
                                0,
                                0);
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b)
    {
        int count = getChildCount();

        for (int i = 0; i < count; i++) {
            View child = getChildAt(i);
            if (child.getVisibility() != GONE) {
                QtLayout.LayoutParams lp =
                        (QtLayout.LayoutParams) child.getLayoutParams();

                int childLeft = lp.x;
                int childTop = lp.y;
                child.layout(childLeft, childTop,
                        childLeft + child.getMeasuredWidth(),
                        childTop + child.getMeasuredHeight());

            }
        }
    }

    // Override to allow type-checking of LayoutParams.
    @Override
    protected boolean checkLayoutParams(ViewGroup.LayoutParams p)
    {
        return p instanceof QtLayout.LayoutParams;
    }

    @Override
    protected ViewGroup.LayoutParams generateLayoutParams(ViewGroup.LayoutParams p)
    {
        return new LayoutParams(p);
    }

    /**
    * Per-child layout information associated with AbsoluteLayout.
    * See
    * {@link android.R.styleable#AbsoluteLayout_Layout Absolute Layout Attributes}
    * for a list of all child view attributes that this class supports.
    */
    public static class LayoutParams extends ViewGroup.LayoutParams
    {
        /**
        * The horizontal, or X, location of the child within the view group.
        */
        public int x;
        /**
        * The vertical, or Y, location of the child within the view group.
        */
        public int y;

        /**
        * Creates a new set of layout parameters with the specified width,
        * height and location.
        *
        * @param width the width, either {@link #FILL_PARENT},
                {@link #WRAP_CONTENT} or a fixed size in pixels
        * @param height the height, either {@link #FILL_PARENT},
                {@link #WRAP_CONTENT} or a fixed size in pixels
        * @param x the X location of the child
        * @param y the Y location of the child
        */
        public LayoutParams(int width, int height, int x, int y)
        {
            super(width, height);
            this.x = x;
            this.y = y;
        }

        /**
        * {@inheritDoc}
        */
        public LayoutParams(ViewGroup.LayoutParams source)
        {
            super(source);
        }
    }

    public void moveChild(View view, int index)
    {
        if (view == null)
            return;

        if (indexOfChild(view) == -1)
            return;

        detachViewFromParent(view);
        requestLayout();
        invalidate();
        attachViewToParent(view, index, view.getLayoutParams());
    }

    /**
    * set the layout params on a child view.
    *
    * Note: This function adds the child view if it's not in the
    *       layout already.
    */
    public void setLayoutParams(final View childView,
                                final ViewGroup.LayoutParams params,
                                final boolean forceRedraw)
    {
        // Invalid view
        if (childView == null)
            return;

        // Invalid params
        if (!checkLayoutParams(params))
            return;

        // View is already in the layout and can therefore be updated
        final boolean canUpdate = (this == childView.getParent());

        if (canUpdate) {
            childView.setLayoutParams(params);
            if (forceRedraw)
                invalidate();
        } else {
            addView(childView, params);
        }
    }
}
