/* This file is part of the KDE libraries
  Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/
#ifndef __OTHEMEBASE_H
#define __OTHEMEBASE_H

/* OPIE */
#include <opie2/odebug.h>
using namespace Opie::Core;

/* QT */
#include <qdatetime.h>
#include <qtimer.h>
#include <qwindowsstyle.h>
#include <qpixmap.h>
#include <qpe/config.h>
#include <qimage.h>
#include <qintcache.h>
#include <qstring.h>
#include <qpalette.h>

#define WIDGETS 54

/**
 * This class adds simple time management to QPixmap for use in flushing
 * OThemeCache.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 */
class OThemePixmap : public QPixmap
{
public:
    enum BorderType{Top = 0, Bottom, Left, Right, TopLeft, TopRight, BottomLeft,
                    BottomRight};

    OThemePixmap( bool timer = true );
    OThemePixmap( const OThemePixmap &p );
    ~OThemePixmap();
    QPixmap* border( BorderType type );
    void setBorder( BorderType type, const QPixmap &p );
    void updateAccessed();
    bool isOld();
protected:
    QTime *t;
    QPixmap *b[ 8 ];

private:
    class OThemePixmapPrivate;
    OThemePixmapPrivate *d;
};

inline QPixmap* OThemePixmap::border( BorderType type )
{
    return ( b[ type ] );
}

inline void OThemePixmap::setBorder( BorderType type, const QPixmap &p )
{
    if ( b[ type ] ) {
        owarn << "OThemePixmap: Overwriting existing border!" << oendl;
        delete( b[ type ] );
    }
    b[ type ] = new QPixmap( p );
}

inline void OThemePixmap::updateAccessed()
{
    if (t)
        t->start();
}

inline bool OThemePixmap::isOld()
{
    return ( t ? t->elapsed() >= 300000 : false );
}

/**
 * A very simple pixmap cache for theme plugins. QPixmapCache is not used
 * since it uses QString keys which are not needed. All the information we
 * need can be encoded in a numeric key. Using QIntCache instead allows us to
 * skip the string operations.
 *
 * This class is mostly just inline methods that do bit operations on a key
 * composed of the widget ID, width and/or height, and then calls
 * QIntCache::find().
 *
 * One other thing to note is that full, horizontal, and vertically scaled
 * pixmaps are not used interchangeably. For example, if you insert a fully
 * scaled pixmap that is 32x32 then request a horizontally scaled pixmap with
 * a width of 32, they will not match. This is because a pixmap that has been
 * inserted into the cache has already been scaled at some point and it is
 * very likely the vertical height was not originally 32. Thus the pixmap
 * will be wrong when drawn, even though the horizontal width matches.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 *
 */
class OThemeCache : public QObject
{
    Q_OBJECT
public:
    /**
     * The scale hints supported by the cache. Note that Tiled is not here
     * since tiled pixmaps are kept only once in OThemeBase.
     */
    enum ScaleHint{FullScale, HorizontalScale, VerticalScale};
    /**
     * The constructor.
     *
     * @param maxSize The maximum size of the cache in kilobytes.
     */
    OThemeCache( int maxSize, QObject *parent = 0, const char *name = 0 );
    /**
     * Inserts a new pixmap into the cache.
     *
     * @param pixmap The pixmap to insert.
     * @param scale The scaling type of the pixmap.
     * @param widgetID The widget ID of the pixmap, usually from OThemeBase's
     * WidgetType enum.
     *
     * @return True if the insert was successful, false otherwise.
     */
    bool insert( OThemePixmap *pixmap, ScaleHint scale, int widgetID,
                 bool border = false, bool mask = false );
    /**
     * Returns a fully scaled pixmap.
     *
     * @param w The pixmap width to search for.
     * @param h The pixmap height to search for.
     * @param widgetID The widget ID to search for.
     *
     * @return True if a pixmap matching the width, height, and widget ID of
     * the pixmap exists, NULL otherwise.
     */
    OThemePixmap* pixmap( int w, int h, int widgetID, bool border = false,
                          bool mask = false );
    /**
     * Returns a horizontally scaled pixmap.
     *
     * @param w The pixmap width to search for.
     * @param widgetID The widget ID to search for.
     *
     * @return True if a pixmap matching the width and widget ID of
     * the pixmap exists, NULL otherwise.
     */
    OThemePixmap* horizontalPixmap( int w, int widgetID );
    /**
     * Returns a vertically scaled pixmap.
     *
     * @param h The pixmap height to search for.
     * @param widgetID The widget ID to search for.
     *
     * @return True if a pixmap matching the height and widget ID of
     * the pixmap exists, NULL otherwise.
     */
    OThemePixmap* verticalPixmap( int h, int widgetID );
protected slots:
    void flushTimeout();
protected:
    QIntCache<OThemePixmap> cache;
    QTimer flushTimer;

private:
    class OThemeCachePrivate;
    OThemeCachePrivate *d;
};


/**
 * This is a base class for KDE themed styles. It implements a cache,
 * configuration file parsing, pixmap scaling, gradients, and a lot
 * of inline methods for accessing user specified parameters.
 *
 * Note that this class *does not* actually implement any themes. It just
 * provides the groundwork for doing so. The only reason to use this class
 * directly is if you plan to reimplement all of the widgets. Otherwise,
 * refer to OThemeStyle for a fully themed style you can derive from.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 */
class OThemeBase: public QWindowsStyle
{
    Q_OBJECT
public:
    /**
     * Constructs a new OThemeBase object.
     */
    OThemeBase( const QString &configFile );
    ~OThemeBase();
    /**
     * Describes if a pixmap should be scaled fully, horizontally, vertically,
     * or not at all and tiled.
     */
    enum ScaleHint{FullScale, HorizontalScale, VerticalScale, TileScale};
    /**
     * The default arrow types.
     */
    enum ArrowStyle{MotifArrow, LargeArrow, SmallArrow};
    /**
     * The default frame shading styles.
     */
    enum ShadeStyle{Motif, Windows, Next, KDE};
    /**
     * The default scrollbar button layout. BottomLeft is like what Next
     * uses, BottomRight is like Platinum, and Opposite it like Windows and
     * Motif.
     */
    enum SButton{SBBottomLeft, SBBottomRight, SBOpposite};
    /**
     * The gradient types. Horizontal is left to right, Vertical is top to
     * bottom, and diagonal is upper-left to bottom-right.
     */
    enum Gradient{GrNone, GrHorizontal, GrVertical, GrDiagonal, GrPyramid,
                  GrRectangle, GrElliptic, GrReverseBevel};
    /**
     * This provides a list of widget types that OThemeBase recognizes.
     */
    /* Internal note: The order here is important. Some widgets inherit
     * properties. This is usually for when you have two settings for the
     * same widget, ie: on(sunken), and off. The on settings will inherit
     * the properties of the off one when nothing is specified in the config.
     *
     * In order to be able to handle this while still having everything in
     * one group that is easy to loop from we have the following order:
     * unsunked(off) items, sunken(on)items, and then the ones that don't
     * matter. INHERIT_ITEMS define the number of widgets that have inheritence
     * so if 0 == PushButtonOff then INHERIT_ITEMS should == PushButtonOn
     * and so on. WIDGETS define the total number of widgets.
     */
    enum WidgetType{
        // Off (unsunken widgets)
        PushButton = 0, ComboBox, HScrollBarSlider, VScrollBarSlider, Bevel,
        ToolButton, ScrollButton, HScrollDeco, VScrollDeco,
        ComboDeco, MenuItem, InactiveTab, ArrowUp, ArrowDown, ArrowLeft,
        ArrowRight,
        // On (sunken widgets)
        PushButtonDown, ComboBoxDown, HScrollBarSliderDown,
        VScrollBarSliderDown, BevelDown, ToolButtonDown, ScrollButtonDown,
        HScrollDecoDown, VScrollDecoDown, ComboDecoDown, MenuItemDown,
        ActiveTab, SunkenArrowUp, SunkenArrowDown, SunkenArrowLeft,
        SunkenArrowRight,
        // Everything else (indicators must have separate settings)
        HScrollGroove, VScrollGroove, Slider, SliderGroove, IndicatorOn,
        IndicatorOff, ExIndicatorOn, ExIndicatorOff, HBarHandle, VBarHandle,
        ToolBar, Splitter, CheckMark, MenuBar, DisArrowUp, DisArrowDown,
        DisArrowLeft, DisArrowRight, ProgressBar, ProgressBg, MenuBarItem,
        Background};

    /**
     * The scaling type specified by the Config file.
     *
     * @param widget A Widgets enum value.
     *
     * @return A ScaleHint enum value.
     */
    ScaleHint scaleHint( WidgetType widget ) const;
    /**
     * The gradient type specified by the KConfig file.
     *
     * @param widget A Widgets enum value.
     *
     * @return A Gradient enum value.
     */
    Gradient gradientHint( WidgetType widget ) const;
    /**
     * The color group specified for a given widget.
     * If a color group is set in the theme configuration
     * that is used, otherwise defaultColor is returned.
     *
     * @param defaultColor The colorGroup to set if one is available.
     *
     * @param widget The widget whose color group to retrieve.
     *
     */
    const QColorGroup* colorGroup( const QColorGroup &defaultGroup,
                                   WidgetType widget ) const;

    QBrush pixmapBrush( const QColorGroup &group, QColorGroup::ColorRole role,
                        int w, int h, WidgetType widget );
    /**
     * True if the widget has a pixmap or gradient specified.
     */
    bool isPixmap( WidgetType widget ) const;
    /**
     * True if the widget has a color group specified.
     */
    bool isColor( WidgetType widget ) const;
    /**
     * True if the user specified a 3D focus rectangle
     */
    bool is3DFocus() const;
    /**
     * If the user specified a 3D focus rectangle, they may also specify an
     * offset from the default rectangle to use when drawing it. This returns
     * the specified offset.
     */
    int focusOffset() const;
    /**
     * The border width of the specified widget.
     */
    int borderWidth( WidgetType widget ) const;
    /**
     * Pixmap border width of the specified widget.
     */
    int pixBorderWidth( WidgetType widget ) const;
    /**
     * Returns the border pixmap if enabled for the specified widget. This
     * will contain the originial pixmap, plus the edges separated in
     * OThemePixmap::border() if valid. If invalid it will return NULL.
     */
    OThemePixmap* borderPixmap( WidgetType widget );
    /**
     * The highlight width of the specified widget.
     */
    int highlightWidth( WidgetType widget ) const;
    /**
     * The border plus highlight width of the widget.
     */
    int decoWidth( WidgetType widget ) const;
    /**
     * The extent (width for vertical, height for horizontal) requested
     * for the scrollbars.
     */
    int getSBExtent() const;
    /**
     * The scrollbar button layout.
     */
    SButton scrollBarLayout() const;
    /**
     * The arrow type.
     */
    ArrowStyle arrowType() const;
    /**
     * The shading type.
     */
    ShadeStyle shade() const;
    /**
     * The frame width.
     */
    int frameWidth() const;
    /**
     * The splitter width.
     */
    int splitWidth() const;
    /**
     * The contrast for some bevel effects such as reverse gradient.
     */
    int bevelContrast( WidgetType widget ) const;
    /**
     * The button text X shift.
     */
    int buttonXShift() const;
    /**
     * The button text Y shift.
     */
    int buttonYShift() const;
    /**
     * Returns either the slider length of the slider pixmap if available,
     * otherwise the length specified in the config file.
     */
    int sliderButtonLength() const;
    /**
     * True if rounded buttons are requested.
     */
    bool roundButton() const;
    /**
     * True if rounded comboboxes are requested.
     */
    bool roundComboBox() const;
    /**
     * True if rounded slider grooves are requested.
     */
    bool roundSlider() const;
    /**
     * True if a line should be drawn on the bottom of active tabs.
     */
    bool activeTabLine() const;
    /**
     * True if a line should be drawn on the bottom of inactive tabs.
     */
    bool inactiveTabLine() const;
    /**
     * Returns the current uncached pixmap for the given widget. This will
     * usually be either the last scaled or gradient pixmap if those have
     * been specified in the config file, the original pixmap if not, or NULL
     * if no pixmap has been specified.
     */
    OThemePixmap* uncached( WidgetType widget ) const;
    /**
     * Returns the pixmap for the given widget at the specified width and
     * height. This will return NULL if no pixmap or gradient is specified.
     * It may also return a different sized pixmap if the scaling
     * is set to Tiled. When using this method, you should call it using
     * the needed width and height then use QPainter::drawTiledPixmap to
     * paint it. Doing this, if the pixmap is scaled it will be the proper
     * size, otherwise it will be tiled.
     *
     * @param w Requested width.
     * @param h Requested height.
     * @param widget Widget type.
     * @return The pixmap or NULL if one is not specified.
     */
    virtual OThemePixmap *scalePixmap( int w, int h, WidgetType widget );
    /**
     * This method reads a configuration file and applies it to the user's
     * kstylerc file. It does not signal applications to reload via the
     * KDEChangeGeneral atom, if you want to do this you must do so yourself.
     * See kcmdisplay's general.cpp for an example.
     *
     * @param file The configuration file to apply.
     */
    static void applyConfigFile( const QString &file );
protected:
    /**
     * Returns a QImage for the given widget if the widget is scaled, NULL
     * otherwise. QImages of the original pixmap are stored for scaled
     * widgets in order to facilitate fast and accurate smooth-scaling. This
     * also saves us a conversion from a pixmap to an image then back again.
     */
    QImage* image( WidgetType widget ) const;
    /**
     * Returns the gradient high color if one is specified, NULL otherwise.
     */
    QColor* gradientHigh( WidgetType widget ) const;
    /**
     * Returns the gradient low color if one is specified, NULL otherwise.
     */
    QColor* gradientLow( WidgetType widget ) const;
    /**
     * Reads in all the configuration file entries supported.
     *
     * @param colorStyle The style for the color groups. In KDE, colors were
     * calculated a little differently for Motif vs Windows styles. This
     * is obsolete.
     */
    void readConfig( Qt::GUIStyle colorStyle = Qt::WindowsStyle );
    void readWidgetConfig( int i, Config *config, QString *pixnames,
                           QString *brdnames, bool *loadArray );
    void copyWidgetConfig( int sourceID, int destID, QString *pixnames,
                           QString *brdnames );
    /**
     * Makes a full color group based on the given foreground and background
     * colors. This is the same code used by KDE (kapp.cpp) in previous
     * versions.
     */
    QColorGroup* makeColorGroup( QColor &fg, QColor &bg,
                                 Qt::GUIStyle style = Qt::WindowsStyle );
    OThemePixmap* scale( int w, int h, WidgetType widget );
    OThemePixmap* scaleBorder( int w, int h, WidgetType type );
    OThemePixmap* gradient( int w, int h, WidgetType widget );
    OThemePixmap* blend( WidgetType widget );
    void generateBorderPix( int i );
    void applyResourceGroup( Config *config, int i, QString *copyfrom, QString *pixnames, QString *brdnames );
    void applyMiscResourceGroup( Config *config );
    void readResourceGroup( int i, QString *copyfrom, QString *pixnames, QString *brdnames,
                            bool *loadArray );
    void readMiscResourceGroup();
    /**
     * Attempts to load a pixmap from the default OThemeBase locations.
     */
    OThemePixmap* loadPixmap( QString &name );
    /**
     * Attempts to load a image from the default OThemeBase locations.
     */
    QImage* loadImage( QString &name );
private:
    SButton sbPlacement;
    ArrowStyle arrowStyle;
    ShadeStyle shading;
    int defaultFrame;
    int btnXShift, btnYShift;
    int sliderLen;
    int splitterWidth;
    int focus3DOffset;
    int sbExtent;
    bool smallGroove;
    bool roundedButton, roundedCombo, roundedSlider;
    bool aTabLine, iTabLine;
    bool focus3D;
    OThemeCache *cache;
    int cacheSize;
    QString configFileName;
    QString configFilePath;

protected:
    QColor fgcolor, bgcolor, selfgcolor, selbgcolor, winfgcolor, winbgcolor;

private:
    /**
     * The theme pixmaps. Many of these may be NULL if no pixmap is specified.
     * There may also be duplicate pixmap pointers if more than one widget
     * uses the same tiled pixmap. If a pixmap is tiled, it is kept here and
     * this acts as a cache. Otherwise this will hold whatever the last scaled
     * pixmap was.
     */
    OThemePixmap *pixmaps[ WIDGETS ];
    /**
     * The theme images. These are for scaled images and are kept in order
     * to maintain fast smoothscaling.
     */
    QImage *images[ WIDGETS ];
    /**
     * The border widths
     */
    unsigned char borders[ WIDGETS ];
    /**
     * The highlight widths
     */
    unsigned char highlights[ WIDGETS ];
    /**
     * The scale hints for pixmaps and gradients.
     */
    ScaleHint scaleHints[ WIDGETS ];
    /**
     * All the color groups.
     */
    QColorGroup *colors[ WIDGETS ];
    /**
     * Gradient low colors (or blend background).
     */
    QColor *grLowColors[ WIDGETS ];
    /**
     * Gradient high colors.
     */
    QColor *grHighColors[ WIDGETS ];
    /**
     * Gradient types.
     */
    Gradient gradients[ WIDGETS ];
    /**
     * Blend intensity factors
     */
    float blends[ WIDGETS ];
    /**
     * Bevel contrasts
     */
    unsigned char bContrasts[ WIDGETS ];
    /**
     * Duplicate pixmap entries (used during destruction).
     */
    bool duplicate[ WIDGETS ];
    /**
     * Pixmapped border widths
     */
    int pbWidth[ WIDGETS ];
    /**
     * Pixmapped borders
     */
    OThemePixmap *pbPixmaps[ WIDGETS ];
    /**
     * Duplicate border pixmapped border entries
     */
    bool pbDuplicate[ WIDGETS ];

private:
    class OThemeBasePrivate;
    OThemeBasePrivate *d;

};

inline bool OThemeBase::isPixmap( WidgetType widget ) const
{
    return ( pixmaps[ widget ] != NULL || gradients[ widget ] != GrNone );
}

inline bool OThemeBase::isColor( WidgetType widget ) const
{
    return ( colors[ widget ] != NULL );
}

inline bool OThemeBase::is3DFocus() const
{
    return ( focus3D );
}

inline int OThemeBase::focusOffset() const
{
    return ( focus3DOffset );
}

inline int OThemeBase::bevelContrast( WidgetType widget ) const
{
    return ( bContrasts[ widget ] );
}

inline OThemeBase::ScaleHint OThemeBase::scaleHint( WidgetType widget ) const
{
    return ( ( widget < WIDGETS ) ? scaleHints[ widget ] : TileScale );
}

inline OThemeBase::Gradient OThemeBase::gradientHint( WidgetType widget ) const
{
    return ( ( widget < WIDGETS ) ? gradients[ widget ] : GrNone );
}

inline OThemePixmap* OThemeBase::uncached( WidgetType widget ) const
{
    return ( pixmaps[ widget ] );
}

inline QBrush OThemeBase::pixmapBrush( const QColorGroup &group,
                                       QColorGroup::ColorRole role,
                                       int w, int h, WidgetType widget )
{
    if ( pixmaps[ widget ] || images[ widget ] )
        return ( QBrush( group.color( role ), *scalePixmap( w, h, widget ) ) );
    else
        return ( group.color( role ) );
}

inline const QColorGroup* OThemeBase::colorGroup( const QColorGroup &defaultGroup,
        WidgetType widget ) const
{
    return ( ( colors[ widget ] ) ? colors[ widget ] : &defaultGroup );
}

inline int OThemeBase::borderWidth( WidgetType widget ) const
{
    return ( pbWidth[ widget ] ? pbWidth[ widget ] : borders[ widget ] );
}

inline int OThemeBase::pixBorderWidth( WidgetType widget ) const
{
    return ( pbWidth[ widget ] );
}

inline int OThemeBase::highlightWidth( WidgetType widget ) const
{
    return ( pbWidth[ widget ] ? 0 : highlights[ widget ] );
}

inline int OThemeBase::decoWidth( WidgetType widget ) const
{
    return ( pbWidth[ widget ] ? pbWidth[ widget ] : borders[ widget ] + highlights[ widget ] );
}

inline QColor* OThemeBase::gradientHigh( WidgetType widget ) const
{
    return ( grHighColors[ widget ] );
}

inline QColor* OThemeBase::gradientLow( WidgetType widget ) const
{
    return ( grLowColors[ widget ] );
}

inline QImage* OThemeBase::image( WidgetType widget ) const
{
    return ( images[ widget ] );
}

inline OThemeBase::SButton OThemeBase::scrollBarLayout() const
{
    return ( sbPlacement );
}

inline OThemeBase::ArrowStyle OThemeBase::arrowType() const
{
    return ( arrowStyle );
}

inline OThemeBase::ShadeStyle OThemeBase::shade() const
{
    return ( shading );
}

inline int OThemeBase::frameWidth() const
{
    return ( defaultFrame );
}

inline int OThemeBase::buttonXShift() const
{
    return ( btnXShift );
}

inline int OThemeBase::splitWidth() const
{
    return ( splitterWidth );
}

inline int OThemeBase::buttonYShift() const
{
    return ( btnYShift );
}

inline int OThemeBase::sliderButtonLength() const
{
    if ( isPixmap( Slider ) )
        return ( uncached( Slider ) ->width() );
    else
        return ( sliderLen );
}

inline bool OThemeBase::roundButton() const
{
    return ( roundedButton );
}

inline bool OThemeBase::roundComboBox() const
{
    return ( roundedCombo );
}

inline bool OThemeBase::roundSlider() const
{
    return ( roundedSlider );
}

inline bool OThemeBase::activeTabLine() const
{
    return ( aTabLine );
}

inline bool OThemeBase::inactiveTabLine() const
{
    return ( iTabLine );
}

inline int OThemeBase::getSBExtent() const
{
    return ( sbExtent );
}

inline OThemePixmap* OThemeBase::borderPixmap( WidgetType widget )
{
    return ( pbPixmaps[ widget ] );
}

#endif
