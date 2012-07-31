#include "UIMain.h"
#include "ui_UIMain.h"
#include "CodeEditor.h"
#include "MarginStacker.h"
#include "SyntaxFactory.h"
#include "QodeEdit.h"
#include "SyntaxHighlighter.h"
#include "TextDocument.h"

// QodeEditor

QodeEditor::QodeEditor( QWidget* parent )
    : CodeEditor( parent )
{
    setCaretLineBackground( QColor( 150, 150, 150, 150 ) );
    
    MarginStacker* margins = new MarginStacker( this );
    margins->setVisible( MarginStacker::LineBookmark, true );
    margins->setVisible( MarginStacker::LineNumber, true );
    margins->setVisible( MarginStacker::LineRevision, true );
    margins->setVisible( MarginStacker::LineFold, true );
    margins->setVisible( MarginStacker::LineSpacing, true );
    
    // fake save document shortcut
    new QShortcut( QKeySequence::Save, this, SLOT( save() ) );
}

QString QodeEditor::fileContent( const QString& filePath, const QByteArray& textCodec )
{
    QFile file( filePath );
    
    if ( !file.exists() ) {
        return QString::null;
    }
    
    if ( !file.open( QIODevice::ReadOnly ) ) {
        return QString::null;
    }
    
    QTextCodec* codec = QTextCodec::codecForName( textCodec );
    
    if ( !codec ) {
        codec = QTextCodec::codecForLocale();
    }
    
    return codec->toUnicode( file.readAll() );
}

void QodeEditor::save()
{
    document()->setModified( false );
}

// SpacerWidget

class SpacerWidget : public QWidget {
public:
    SpacerWidget( QWidget* parent = 0 )
        : QWidget( parent )
    {
        setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    }
};

// UIMain

UIMain::UIMain( QWidget* parent )
    : QMainWindow( parent ), ui( new Ui_UIMain )
{
    ui->setupUi( this );
    ui->toolBar->addWidget( new SpacerWidget( this ) );
    ui->toolBar->addWidget( ui->cbSyntax );
    
    QString error;
    
    if ( Syntax::Factory::load( &error ) ) {
        error = "Syntaxes loaded correctly";
    }
    
    //qWarning() << Syntax::Factory::availableSyntaxes();
    
    /*".desktop", "4GL", "4GL-PER", "ABAP", "ABC", "ActionScript 2.0", "Ada", --
    "AHDL", "Alerts", "Alerts_indent", "AMPLE", "ANS-Forth94", "ANSI C89", "Ansys",
    "Apache Configuration", "Asm6502", "ASN.1", "ASP", "Asterisk", "AVR Assembler",
    "AWK", "B-Method", "Bash", "BibTeX", "Boo", "C", "C#", "C++", "Cg", "CGiS", "ChangeLog",
    "Cisco", "Clipper", "Clojure", "CMake", "CoffeeScript", "ColdFusion", "Common Lisp",
    "Component-Pascal", "Crack", "CSS", "CUE Sheet", "D", "Debian Changelog", "Debian Control",
    "Diff", "Django HTML Template", "dot", "Doxygen", "DoxygenLua", "DTD", "E Language", "Eiffel",
    "Email", "Erlang", "Euphoria", "ferite", "Fortran", "FreeBASIC", "FSharp", "fstab", "GAP",
    "GDB Backtrace", "GDL", "GlossTex", "GLSL", "GNU Assembler", "GNU Gettext", "GNU Linker Script",
    "GNU M4", "Go", "Haskell", "Haxe", "HTML", "IDL", "ILERPG", "Inform", "INI Files", "Intel x86 (NASM)",
    "Jam", "Java", "Javadoc", "JavaScript", "JSON", "JSP", "KBasic", "KDev-PG[-Qt] Grammar", "LaTeX",
    "LDIF", "Lex/Flex", "LilyPond", "Literate Haskell", "Logtalk", "LPC", "LSL", "Lua", "M3U", "MAB-DB",
    "Makefile", "Mason", "Matlab", "Maxima", "MediaWiki", "MEL", "mergetag text", "Metapost/Metafont",
    "MIPS Assembler", "Modelica", "Modelines", "Modula-2", "MonoBasic", "Motorola 68k (VASM/Devpac)",
    "Motorola DSP56k", "MS-DOS Batch", "Music Publisher", "Nemerle", "noweb", "Objective Caml",
    "Objective-C", "Objective-C++", "Octave", "OORS", "OPAL", "Pango", "Pascal", "Perl", "PGN",
    "PHP/PHP", "PicAsm", "Pig", "Pike", "PostScript", "POV-Ray", "progress", "Prolog", "PureBasic",
    "Python", "QMake", "QML", "Quake Script", "R Script", "RapidQ", "RELAX NG", "RelaxNG-Compact",
    "RenderMan RIB", "reStructuredText", "REXX", "Roff", "RPM Spec", "RSI IDL", "Ruby", "Ruby/Rails/RHTML",
    "Sather", "Scala", "Scheme", "scilab", "SCSS", "sed", "SGML", "Sieve", "SiSU", "SML", "Spice", "SQL",
    "SQL (MySQL)", "SQL (PostgreSQL)", "Stata", "SystemC", "SystemVerilog", "TADS 3", "Tcl/Tk", "Tcsh",
    "Texinfo", "TI Basic", "Troff Mandoc", "txt2tags", "UnrealScript", "Valgrind Suppression", "Velocity",
    "Vera", "Verilog", "VHDL", "VRML", "Wesnoth Markup Language", "WINE Config", "x.org Configuration",
    "xHarbour", "XML", "XML (Debug)", "xslt", "XUL", "yacas", "Yacc/Bison", "YAML", "Zonnon", "Zsh"*/
    
    QDir dir( QodeEdit::sharedDataFilePath( "/samples" ) );
    const QFileInfoList files = dir.entryInfoList( QDir::Files | QDir::NoDotAndDotDot );
    
    foreach ( const QFileInfo& file, files ) {
        const QString filePath = file.absoluteFilePath();
        Syntax::Highlighter* highlighter = Syntax::Factory::highlighterForFilePath( filePath );
        
        if ( highlighter ) {
            QodeEditor* editor = new QodeEditor( this );
            editor->setInitialText( QodeEditor::fileContent( filePath ) );
            editor->textDocument()->setSyntaxHighlighter( highlighter );
            
            QListWidgetItem* item = new QListWidgetItem( ui->lwEditors );
            item->setText( highlighter->syntaxDocument().name );
            item->setData( Qt::UserRole, QVariant::fromValue( editor ) );
            ui->swEditors->addWidget( editor );
        }
        else {
            qWarning( "%s: Can't create highlighter for '%s'", Q_FUNC_INFO, qPrintable( filePath ) );
        }
    }
    
    statusBar()->showMessage( error );
}

UIMain::~UIMain()
{
    delete ui;
    Syntax::Factory::free();
}

QodeEditor* UIMain::editor( int row ) const
{
    QListWidgetItem* item = ui->lwEditors->item( row );
    
    if ( !item ) {
        Q_ASSERT( item );
        return 0;
    }
    
    return item->data( Qt::UserRole ).value<QodeEditor*>();
}

void UIMain::on_lwEditors_currentRowChanged( int row )
{
    if ( ui->swEditors->currentIndex() != row ) {
        ui->swEditors->setCurrentIndex( row );
    }
}

void UIMain::on_swEditors_currentChanged( int row )
{
    if ( ui->lwEditors->currentRow() != row ) {
        ui->lwEditors->setCurrentRow( row );
    }
    
    ui->cbSyntax->setCurrentSyntax( editor( row )->textDocument()->syntaxHighlighter()->syntaxDocument().name );
}
