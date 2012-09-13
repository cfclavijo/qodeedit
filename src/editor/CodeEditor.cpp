/****************************************************************************
**
**         Created using Monkey Studio IDE v1.9.0.1 (1.9.0.1)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com> and The QodeEdit development team
** Project   : QodeEdit
** FileName  : CodeEditor.cpp
** Date      : 2012-09-02T23:04:46
** License   : LGPL3
** Comment   : A source code component editor based on QPlainTextEdit/QTextDocument/QSyntaxHighlighter and Kate xml language definition files.
** Home Page : https://github.com/pasnox/qodeedit
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "CodeEditor.h"
#include "TextDocument.h"
#include "TextBlockUserData.h"
#include "margin/MarginStacker.h"
#include "margin/AbstractMargin.h"

#include <QStyleOptionFrameV3>
#include <QTextBlock>
#include <QPainter>
#include <QDebug>

// CodeEditorPrivate

class CodeEditorPrivate {
public:
    CodeEditor* editor;
    MarginStacker* stacker;
    QPalette originalPalette;
    QodeEdit::Ruler rulerMode;
    int rulerWidth;
    
    CodeEditorPrivate( CodeEditor* _editor )
            : editor( _editor ),
            stacker( 0 ),
            originalPalette( _editor->palette() ),
            rulerMode( QodeEdit::NoRuler ),
            rulerWidth( 80 )
    {
        Q_ASSERT( editor );
    }

    QLine rulerLine() const {
        const QPoint offset = editor->contentOffset().toPoint();
        const int x = rulerWidth *QFontMetrics( editor->font() ).averageCharWidth();
        return QLine( QPoint( x +offset.x(), 0 ), QPoint( x +offset.x(), editor->viewport()->height() ) );
    }
    
    QRect rulerRect() const {
        const QPoint offset = editor->contentOffset().toPoint();
        const int x = rulerWidth *QFontMetrics( editor->font() ).averageCharWidth();
        QRect rect( QPoint( x +offset.x(), 0 ), QSize( editor->viewport()->size() -QSize( x +offset.x(), 0 ) ) );
        return rect;
    }
    
    QRect caretLineRect() const {
        QRect rect = editor->cursorRect()/*.adjusted( 0, -1, 0, 1 )*/;
        rect.setX( 0 );
        rect.setWidth( editor->viewport()->width() );
        return rect;
    }
    
    void paintFrame() {
        QPainter painter( editor );
        QStyleOptionFrameV3 option;
        
        option.initFrom( editor );
        option.palette = originalPalette;
        option.rect = editor->frameRect();
        option.frameShape = editor->frameShape();
        
        switch ( option.frameShape ) {
            case QFrame::Box:
            case QFrame::HLine:
            case QFrame::VLine:
            case QFrame::StyledPanel:
            case QFrame::Panel:
                option.lineWidth = editor->lineWidth();
                option.midLineWidth = editor->midLineWidth();
                break;
            default:
                // most frame styles do not handle customized line and midline widths
                // (see updateFrameWidth()).
                option.lineWidth = editor->frameWidth();
                break;
        }

        if ( editor->frameShadow() == QFrame::Sunken ) {
            option.state |= QStyle::State_Sunken;
        }
        else if ( editor->frameShadow() == QFrame::Raised ) {
            option.state |= QStyle::State_Raised;
        }

        editor->style()->drawControl( QStyle::CE_ShapedFrame, &option, &painter, editor );
    }
};

// CodeEditor

CodeEditor::CodeEditor( QWidget* parent )
    : QPlainTextEdit( parent ),
        d( new CodeEditorPrivate( this ) )
{
    setTextDocument( new TextDocument( this ) );
    setAutoFillBackground( true );
    setCaretLineBackground( caretLineBackground().color().lighter( 200 ) );
    
    connect( this, SIGNAL( cursorPositionChanged() ), viewport(), SLOT( update() ) );
}

CodeEditor::~CodeEditor()
{
    delete d;
}

TextDocument* CodeEditor::textDocument() const
{
    return qobject_cast<TextDocument*>( document() );
}

void CodeEditor::setTextDocument( TextDocument* document )
{
    setDocument( document );
}

MarginStacker* CodeEditor::marginStacker() const
{
    return d->stacker;
}

void CodeEditor::setMarginStacker( MarginStacker* marginStacker )
{
    if ( d->stacker == marginStacker ) {
        return;
    }
    
    if ( d->stacker ) {
        d->stacker->deleteLater();
    }
    
    d->stacker = marginStacker;
    
    if ( d->stacker ) {
        d->stacker->setEditor( this );
    }
}

QString CodeEditor::text() const
{
    return textDocument()->text();
}

void CodeEditor::setText( const QString& text )
{
    textDocument()->setText( text );
    moveCursor( QTextCursor::Start, QTextCursor::MoveAnchor );
}

void CodeEditor::setInitialText( const QString& text )
{
    textDocument()->setInitialText( text );
    moveCursor( QTextCursor::Start, QTextCursor::MoveAnchor );
}

QPoint CodeEditor::cursorPosition() const
{
    const QTextCursor cursor = textCursor();
    return cursor.isNull() ? QPoint() : QPoint( cursor.positionInBlock(), cursor.blockNumber() );
}

void CodeEditor::setCursorPosition( const QPoint& pos )
{
    const QTextBlock block = textDocument()->findBlockByLineNumber( pos.y() );
    const int position = block.position() +( pos.x() < block.length() ? pos.x() : 0 );
    QTextCursor cursor = textCursor();
    cursor.setPosition( position, QTextCursor::MoveAnchor );
    setTextCursor( cursor );
}

int CodeEditor::currentLine() const
{
    return cursorPosition().y();
}

void CodeEditor::setCurrentLine( int line )
{
    setCursorPosition( QPoint( currentColumn(), line ) );
}

int CodeEditor::currentColumn() const
{
    return cursorPosition().x();
}

void CodeEditor::setCurrentColumn( int column )
{
    setCursorPosition( QPoint( column, currentLine() ) );
}

QodeEdit::Ruler CodeEditor::rulerMode() const
{
    return d->rulerMode;
}

void CodeEditor::setRulerMode( QodeEdit::Ruler mode )
{
    d->rulerMode = mode;
    viewport()->update();
}

int CodeEditor::rulerWidth() const
{
    return d->rulerWidth;
}

void CodeEditor::setRulerWidth( int width )
{
    d->rulerWidth = width;
    viewport()->update();
}

QBrush CodeEditor::paper() const
{
    return palette().brush( viewport()->backgroundRole() );
}

void CodeEditor::setPaper( const QBrush& brush )
{
    QPalette pal = palette();
    pal.setBrush( viewport()->backgroundRole(), brush );
    setPalette( pal );
}

QBrush CodeEditor::pen() const
{
    return palette().brush( viewport()->foregroundRole() );
}

void CodeEditor::setPen( const QBrush& brush )
{
    QPalette pal = palette();
    pal.setBrush( viewport()->foregroundRole(), brush );
    setPalette( pal );
}

QBrush CodeEditor::selectionBackground() const
{
    return palette().brush( QPalette::Highlight );
}

void CodeEditor::setSelectionBackground( const QBrush& brush )
{
    QPalette pal = palette();
    pal.setBrush( QPalette::Highlight, brush );
    setPalette( pal );
}

QBrush CodeEditor::selectionForeground() const
{
    return palette().brush( QPalette::HighlightedText );
}

void CodeEditor::setSelectionForeground( const QBrush& brush )
{
    QPalette pal = palette();
    pal.setBrush( QPalette::HighlightedText, brush );
    setPalette( pal );
}

QBrush CodeEditor::caretLineBackground() const
{
    return palette().brush( QPalette::Link );
}

void CodeEditor::setCaretLineBackground( const QBrush& brush )
{
    QPalette pal = palette();
    pal.setBrush( QPalette::Link, brush );
    setPalette( pal );
}

QBrush CodeEditor::caretLineForeground() const
{
    return palette().brush( QPalette::LinkVisited );
}

void CodeEditor::setCaretLineForeground( const QBrush& brush )
{
    QPalette pal = palette();
    pal.setBrush( QPalette::LinkVisited, brush );
    setPalette( pal );
}

bool CodeEditor::hasBookmark( const QTextBlock& block ) const
{
    const TextBlockUserData* data = textDocument()->testUserData( block );
    return data ? data->hasBookmark : false;
}

void CodeEditor::setBookmark( const QTextBlock& block, bool set )
{
    const TextDocument* document = textDocument();
    TextBlockUserData* data = document->testUserData( block );
    
    if ( !data && !set ) {
        return;
    }
    
    data = document->userData( *const_cast<QTextBlock*>( &block ) );
    data->hasBookmark = set;
    
    if ( d->stacker ) {
        AbstractMargin* margin = d->stacker->margin( QodeEdit::BookmarkMargin );
        
        if ( margin ) {
            margin->updateLineRect( block.blockNumber() );
        }
    }
}

void CodeEditor::toggleBookmark( const QTextBlock& block )
{
    setBookmark( block, !hasBookmark( block ) );
}

bool CodeEditor::hasBookmark( int line ) const
{
    return hasBookmark( textDocument()->findBlockByNumber( line ) );
}

void CodeEditor::setBookmark( int line, bool set )
{
    setBookmark( textDocument()->findBlockByNumber( line ), set );
}

void CodeEditor::toggleBookmark( int line )
{
    toggleBookmark( textDocument()->findBlockByNumber( line ) );
}

QRect CodeEditor::blockRect( const QTextBlock& block ) const
{
    QRectF rect = blockBoundingGeometry( block );
    rect.moveTopLeft( rect.topLeft() +QPointF( 0, contentOffset().y() ) );
    return rect.toRect();
}

QRect CodeEditor::lineRect( int line ) const
{
    return blockRect( textDocument()->findBlockByNumber( line ) );
}

bool CodeEditor::event( QEvent* event )
{
    switch ( event->type() ) {
        case QEvent::QEvent::Paint:
            d->paintFrame();
            return true;
        default:
            break;
    }
    
    return QPlainTextEdit::event( event );
}

void CodeEditor::paintEvent( QPaintEvent* event )
{
    QPainter painter( viewport() );
    painter.setRenderHint( QPainter::Antialiasing, false );
    
    // draw ruler
    switch ( d->rulerMode ) {
        case QodeEdit::NoRuler:
            break;
        case QodeEdit::LineRuler:
            painter.setPen( QPen( caretLineForeground(), painter.pen().widthF() ) );
            painter.drawLine( d->rulerLine() );
            break;
        case QodeEdit::BackgroundRuler:
            painter.setPen( Qt::NoPen );
            painter.setBrush( caretLineForeground() );
            painter.drawRect( d->rulerRect() );
            break;
    }
    
    // draw caret line
    painter.fillRect( d->caretLineRect(), caretLineBackground() );
    painter.end();
    
    // normal editor painting
    QPlainTextEdit::paintEvent( event );
}
