/*
               =.            This file is part of the OPIE Project
             .=l.            Copyright (c)  2002 <>
           .>+-=
 _;:,     .>    :=|.         This program is free software; you can
.> <`_,   >  .   <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.--   :           the terms of the GNU General Public
.="- .-=="i,     .._         License as published by the Free Software
 - .   .-<_>     .<>         Foundation; either version 2 of the License,
     ._= =}       :          or (at your option) any later version.
    .%`+i>       _;_.
    .i_,=:_.      -<s.       This program is distributed in the hope that
     +  .  -:.       =       it will be useful,  but WITHOUT ANY WARRANTY;
    : ..    .:,     . . .    without even the implied warranty of
    =_        +     =;=|`    MERCHANTABILITY or FITNESS FOR A
  _.=:.       :    :=>`:     PARTICULAR PURPOSE. See the GNU
..}^=.=       =       ;      Library General Public License for more
++=   -.     .`     .:       details.
 :     =  ...= . :.=-
 -.   .:....=;==+<;          You should have received a copy of the GNU
  -_. . .   )=.  =           General Public License along with
    --        :-=`           this library; see the file COPYING.LIB.
                             If not, write to the Free Software Foundation,
                             Inc., 59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.

*/

#include <unistd.h>

#include <qmenubar.h>
#include <qmessagebox.h>
#include <qtoolbar.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qwidgetstack.h>
#include <qaction.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qwhatsthis.h>

#include <qpe/applnk.h>
#include <qpe/config.h>
#include <qpe/ir.h>
#include <qpe/resource.h>
#include <qpe/qpemessagebox.h>
#include <qpe/alarmserver.h>
#include <qpe/timestring.h>
#include <qpe/qpeapplication.h>

#include <opie/orecur.h>
#include <opie/opimnotifymanager.h>
#include <opie/otodoaccessvcal.h>
#include <opie/owidgetstack.h>

#include <opie/oapplicationfactory.h>

#include "quickeditimpl.h"
#include "todotemplatemanager.h"
#include "templateeditor.h"
#include "tableview.h"

#include "textviewshow.h"
#include "todoeditor.h"
#include "mainwindow.h"

OPIE_EXPORT_APP( OApplicationFactory<Todo::MainWindow> )

using namespace Todo;

MainWindow::MainWindow( QWidget* parent,
                        const char* name, WFlags )
    : OPimMainWindow( "Todolist", parent, name, WType_TopLevel | WStyle_ContextHelp )
{
    if (!name)
        setName("todo window");

    m_syncing = false;
    m_showing = false;
    m_counter = 0;
    m_tempManager = new TemplateManager();
    m_tempManager->load();

    initUI();
    initConfig();
    initViews();
    initActions();
    initEditor();
    initShow();
    initTemplate();

    populateTemplates();
    raiseCurrentView();
    QTimer::singleShot(0, this, SLOT(populateCategories() ) );
}
void MainWindow::initTemplate() {
    m_curTempEd = new TemplateEditor( this, templateManager() );
}
void MainWindow::initActions() {

    // Data menu
    m_edit->insertItem(QWidget::tr("New from template"), m_template,
                       -1, 0 );

    QAction* a = new QAction( QWidget::tr("New Task" ), Resource::loadPixmap( "new" ),
                              QString::null, 0, this, 0 );
    connect(a, SIGNAL( activated() ),
            this, SLOT( slotNew() ) );
    a->setWhatsThis( QWidget::tr( "Click here to create a new task." ) );
    a->addTo(m_tool );
    a->addTo(m_edit );

    a = new QAction( QWidget::tr("Edit Task"), Resource::loadIconSet( "edit" ),
                     QString::null, 0, this, 0 );
    connect(a, SIGNAL(activated() ),
            this, SLOT( slotEdit() ) );
    a->setWhatsThis( QWidget::tr( "Click here to modify the current task." ) );
    a->addTo( m_tool );
    a->addTo( m_edit );
    m_editAction = a;

    a = new QAction( QString::null, QWidget::tr("View Task"), 0, this, 0 );
    connect(a, SIGNAL( activated() ),
            this, SLOT( slotShowDetails() ) );
    a->addTo( m_edit );

    m_edit->insertSeparator();

    a = new QAction( QWidget::tr("Delete..."),  Resource::loadIconSet( "trash" ),
                     QString::null, 0, this, 0 );
    connect(a, SIGNAL(activated() ),
            this, SLOT(slotDelete() ) );
    a->setWhatsThis( QWidget::tr( "Click here to remove the current task." ) );
    a->addTo( m_tool );
    a->addTo( m_edit );
    m_deleteAction = a;

    a = new QAction( QString::null, QWidget::tr("Delete all..."), 0, this, 0 );
    connect(a, SIGNAL( activated() ),
            this, SLOT( slotDeleteAll() ) );
    a->addTo(m_edit );
    m_deleteAllAction = a;

    a = new QAction( QString::null, QWidget::tr("Delete completed"),
                     0, this, 0 );
    connect(a, SIGNAL( activated() ),
            this, SLOT( slotDeleteCompleted() ) );
    a->addTo(m_edit );
    a->setEnabled( TRUE );
    m_deleteCompleteAction = a;

    m_edit->insertSeparator();

    a = new QAction( QString::null, QWidget::tr("Duplicate"), 0, this, 0 );
    connect(a, SIGNAL( activated() ),
            this, SLOT( slotDuplicate() ) );
    a->addTo(m_edit );
    m_duplicateAction = a;

    m_edit->insertSeparator();

    if ( Ir::supported() ) {
        a = new QAction( QWidget::tr( "Beam" ), Resource::loadPixmap( "beam" ), QString::null, 0, this, 0 );
        connect( a, SIGNAL( activated() ), this, SLOT( slotBeam() ) );
        a->setWhatsThis( QWidget::tr( "Click here to send the current task to another device." ) );
        a->addTo( m_edit );
        a->addTo( m_tool );
    }

#if 0
    // Options menu
    a = new QAction( QWidget::tr("Find"), Resource::loadIconSet( "mag" ),
                     QString::null, 0, this, 0 );
    connect(a, SIGNAL( activated() ), this, SLOT( slotFind() ) );
    a->addTo( m_options );
    m_findAction = a;


    m_options->insertSeparator();
#endif

    m_completedAction = new QAction( QString::null, QWidget::tr("Show completed tasks"),
                                     0, this, 0, TRUE );
    m_completedAction->addTo( m_options );
    m_completedAction->setOn( showCompleted() );
    connect(m_completedAction, SIGNAL( toggled(bool) ), this, SLOT(slotShowCompleted(bool) ) );

    a = new QAction( QString::null, QWidget::tr("Show only over-due tasks"),
                     0, this, 0, TRUE );
    a->addTo( m_options );
    a->setOn( showOverDue() );
    connect(a, SIGNAL(toggled(bool)), this, SLOT(slotShowDue(bool) ) );

    m_showDeadLineAction = new QAction( QString::null, QWidget::tr("Show task deadlines"),
                                        0, this, 0, TRUE );
    m_showDeadLineAction->addTo( m_options );
    m_showDeadLineAction->setOn( showDeadline() );
    connect(m_showDeadLineAction, SIGNAL(toggled(bool) ), this, SLOT( slotShowDeadLine( bool ) ) );

    m_showQuickTaskAction = new QAction( QString::null, QWidget::tr("Show quick task bar"),
                                     0, this, 0, TRUE );
    m_showQuickTaskAction->addTo( m_options );
    m_showQuickTaskAction->setOn( showQuickTask() );
    connect(m_showQuickTaskAction, SIGNAL( toggled(bool) ), this, SLOT(slotShowQuickTask(bool) ) );

    m_options->insertSeparator();

    m_bar->insertItem( QWidget::tr("Data") ,m_edit );
    m_bar->insertItem( QWidget::tr("Category"),  m_catMenu );
    m_bar->insertItem( QWidget::tr("Options"), m_options );

    m_curQuick = new QuickEditImpl( this, m_quicktask );
    addToolBar( (QPEToolBar *)m_curQuick->widget(), QWidget::tr( "QuickEdit" ), QMainWindow::Top, TRUE );
    m_curQuick->signal()->connect( this, SLOT(slotQuickEntered() ) );

}
/* m_curCat from Config */
void MainWindow::initConfig() {
    Config config( "todo" );
    config.setGroup( "View" );
    m_completed =  config.readBoolEntry( "ShowComplete", TRUE );
    m_curCat = config.readEntry( "Category",    QString::null );
    m_deadline =  config.readBoolEntry( "ShowDeadLine", TRUE);
    m_overdue = config.readBoolEntry("ShowOverDue", FALSE );
    m_quicktask = config.readBoolEntry("ShowQuickTask", TRUE);
}
void MainWindow::initUI() {

    m_stack = new OWidgetStack(this,  "main stack");

    setCentralWidget( m_stack );

    setToolBarsMovable( FALSE );

    QToolBar *menubarholder = new QToolBar( this );
    menubarholder->setHorizontalStretchable( TRUE );
    m_bar = new QMenuBar( menubarholder );

    m_tool = new QToolBar( this );

    /** QPopupMenu */
    m_edit = new QPopupMenu( this );
    m_options = new QPopupMenu( this );
    m_catMenu = new QPopupMenu( this );
    m_template = new QPopupMenu( this );

    m_catMenu->setCheckable( TRUE );
    m_template->setCheckable( TRUE );

    connect(m_catMenu, SIGNAL(activated(int) ),
            this, SLOT(setCategory(int) ) );
    connect(m_template, SIGNAL(activated(int) ),
            this, SLOT(slotNewFromTemplate(int) ) );
}
void MainWindow::initViews() {

    TableView* tableView = new TableView( this, m_stack );
    QWhatsThis::add( tableView, QWidget::tr( "This is a listing of all current tasks.\n\nThe list displays the following information:\n1. Completed - A green checkmark indicates task is completed.  Click here to complete a task.\n2. Priority - a graphical representation of task priority.  Double-click here to modify.\n3. Description - description of task.  Click here to select the task.\n4. Deadline - shows when task is due.  This column can be shown or hidden by selecting Options->'Show task deadlines' from the menu above." ) );
    m_stack->addWidget( tableView,  m_counter++ );
    m_views.append( tableView );
    m_curView = tableView;
    connectBase( tableView );
    /* add QString type + QString configname to
     * the View menu
     * and subdirs for multiple views
     */
}
void MainWindow::initEditor() {
    m_curEdit = new Editor();
}
void MainWindow::initShow() {
    m_curShow = new TextViewShow(this, this);
    m_stack->addWidget( m_curShow->widget() , m_counter++ );
}
MainWindow::~MainWindow() {
    delete templateManager();
}
void MainWindow::connectBase( ViewBase* ) {
    // once templates and signals mix we'll use it again
}
QPopupMenu* MainWindow::contextMenu( int , bool recur ) {
    QPopupMenu* menu = new QPopupMenu();

    m_editAction->addTo( menu );
    m_deleteAction->addTo( menu );
    m_duplicateAction->addTo( menu );

    menu->insertSeparator();

    /*
     * if this event recurs we allow
     * to detach it.
     * remove all
     */
    if ( recur ) {
        ; // FIXME
    }

    return menu;
}
QPopupMenu* MainWindow::options() {
    qWarning("Options");
    return m_options;
}
QPopupMenu* MainWindow::edit() {
    return m_edit;
}
QToolBar* MainWindow::toolbar() {
    return m_tool;
}
OTodoAccess::List MainWindow::list()const {
    return m_todoMgr.list();
}
OTodoAccess::List MainWindow::sorted( bool asc, int sortOrder ) {
    int cat = 0;
    if ( m_curCat != QWidget::tr("All Categories") )
        cat = currentCatId();
    if ( m_curCat == QWidget::tr("Unfiled") )
        cat = -1;

    qWarning(" Category %d %s",  cat, m_curCat.latin1() );

    int filter = 1;

    if (!m_completed )
        filter |= 4;
    if (m_overdue)
        filter |= 2;

    return m_todoMgr.sorted( asc, sortOrder, filter, cat );
}
OTodoAccess::List MainWindow::sorted( bool asc, int sortOrder, int addFilter) {
    int cat = 0;
    if ( m_curCat != QWidget::tr("All Categories") )
        cat = currentCatId();

    if ( m_curCat == QWidget::tr("Unfiled") )
        cat = -1;

    return m_todoMgr.sorted(asc, sortOrder, addFilter,  cat );
}
OTodo MainWindow::event( int uid ) {
    return m_todoMgr.event( uid );
}
bool MainWindow::isSyncing()const {
    return m_syncing;
}
TemplateManager* MainWindow::templateManager() {
    return m_tempManager;
}
Editor* MainWindow::currentEditor() {
    return m_curEdit;
}
TodoShow* MainWindow::currentShow() {
    return m_curShow;
}
void MainWindow::slotReload() {
    m_syncing = FALSE;
    m_todoMgr.reload();
    currentView()->updateView( );
    raiseCurrentView();
}
void MainWindow::closeEvent( QCloseEvent* e ) {
    if (m_stack->visibleWidget() == currentShow()->widget() ) {
        m_showing = false;
        raiseCurrentView();
        e->ignore();
        return;
    }
    /*
     * we should have flushed and now we're still saving
     * so there is no need to flush
     */
    if (m_syncing ) {
        e->accept();
        return;
    }
    bool quit = false;
    if ( m_todoMgr.saveAll() ){
        qWarning("saved");
        quit = true;
    }else {
	if ( QMessageBox::critical( this, QWidget::tr("Out of space"),
				    QWidget::tr("Todo was unable\n"
				       "to save your changes.\n"
				       "Free up some space\n"
				       "and try again.\n"
				       "\nQuit Anyway?"),
				    QMessageBox::Yes|QMessageBox::Escape,
				    QMessageBox::No|QMessageBox::Default)
	     != QMessageBox::No ) {
	    e->accept();
            quit = true;
	}else
	    e->ignore();

    }

    if (quit ) {
        Config config( "todo" );
        config.setGroup( "View" );
        config.writeEntry( "ShowComplete", showCompleted() );
        config.writeEntry( "Category", currentCategory() );
        config.writeEntry( "ShowDeadLine", showDeadline());
        config.writeEntry( "ShowOverDue", showOverDue() );
        config.writeEntry( "ShowQuickTask", showQuickTask() );
        /* save templates */
        templateManager()->save();
        e->accept();
        qApp->quit();
    }
}
void MainWindow::populateTemplates() {
    m_template->clear();
    QStringList list = templateManager()->templates();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
        m_template->insertItem( (*it) );
    }
}
/*
 * slotNewFromTemplate
 * We use the edit widget to do
 * the config but we setUid(1)
 * to get a new uid
 */
/*
 * first we get the name of the template
 * then we will use the TemplateManager
 */
void MainWindow::slotNewFromTemplate( int id ) {
    QString name = m_template->text( id );

    OTodo event = templateManager()->templateEvent( name );
    event = currentEditor()->edit(this,
                                  event );

    if ( currentEditor()->accepted() ) {
        /* assign new todo */
        event.setUid( 1 );
        handleAlarms( OTodo(), event );
        m_todoMgr.add( event );
        currentView()->addEvent( event );

        populateCategories();
    }
    raiseCurrentView();
}
void MainWindow::slotNew() {
    create();
}
void MainWindow::slotDuplicate() {
    if(m_syncing) {
        QMessageBox::warning(this, QWidget::tr("Todo"),
							 QWidget::tr("Data can not be edited, currently syncing"));
        return;
    }
    OTodo ev = m_todoMgr.event( currentView()->current() );
    /* let's generate a new uid */
    ev.setUid(1);
    m_todoMgr.add( ev );

    currentView()->addEvent( ev );
    raiseCurrentView();
}
void MainWindow::slotDelete() {
    if (!currentView()->current() )
        return;

    if(m_syncing) {
	QMessageBox::warning(this, QWidget::tr("Todo"),
							 QWidget::tr("Data can not be edited, currently syncing"));
	return;
    }
    QString strName = currentView()->currentRepresentation();
    if (!QPEMessageBox::confirmDelete(this, QWidget::tr("Todo"), strName ) )
        return;

    handleAlarms( m_todoMgr.event( currentView()->current() ), OTodo() );
    m_todoMgr.remove( currentView()->current() );
    currentView()->removeEvent( currentView()->current() );
    raiseCurrentView();
}
void MainWindow::slotDelete(int uid ) {
    if( uid == 0 ) return;
    if(m_syncing) {
	QMessageBox::warning(this, QWidget::tr("Todo"),
							 QWidget::tr("Data can not be edited, currently syncing"));
	return;
    }
    OTodo to = m_todoMgr.event(uid);
    if (!QPEMessageBox::confirmDelete(this, QWidget::tr("Todo"), to.toShortText() ) )
        return;

    handleAlarms(to, OTodo() );
    m_todoMgr.remove( to.uid() );
    currentView()->removeEvent( to.uid() );
    raiseCurrentView();
}
void MainWindow::slotDeleteAll() {
    if(m_syncing) {
        QMessageBox::warning(this, QWidget::tr("Todo"),
                             QWidget::tr("Data can not be edited, currently syncing"));
        return;
    }


    if ( !QPEMessageBox::confirmDelete( this, QWidget::tr( "Todo" ), QWidget::tr("all tasks?") ) )
        return;

    m_todoMgr.removeAll();
    currentView()->clear();

    raiseCurrentView();
}
void MainWindow::slotDeleteCompleted() {
    if(m_syncing) {
        QMessageBox::warning(this, QWidget::tr("Todo"),
                             QWidget::tr("Data can not be edited, currently syncing"));
        return;
    }

    if ( !QPEMessageBox::confirmDelete( this, QWidget::tr( "Todo" ), QWidget::tr("all completed tasks?") ) )
        return;


    m_todoMgr.removeCompleted();
    currentView()->updateView( );
}
void MainWindow::slotFind() {

}
void MainWindow::slotEdit() {
    slotEdit( currentView()->current() );
}
/*
 * set the category
 */
void MainWindow::setCategory( int c) {
    if ( c <= 0 ) return;


    qWarning("Iterating over cats %d", c );
    for ( unsigned int i = 1; i < m_catMenu->count(); i++ )
        m_catMenu->setItemChecked(i, c == (int)i );

    if (c == 1 ) {
        m_curCat = QString::null;
        setCaption( QWidget::tr("Todo") + " - " + QWidget::tr("All Categories" ) );

    }else if ( c == (int)m_catMenu->count() - 1 ) {
        m_curCat = QWidget::tr("Unfiled");
        setCaption( QWidget::tr("Todo") + " - " + QWidget::tr("Unfiled") );
    }else {
        m_curCat = m_todoMgr.categories()[c-2];
        setCaption( QWidget::tr("Todo") + " - " + m_curCat );
    }
    m_catMenu->setItemChecked( c, true );

    currentView()->setShowCategory( m_curCat );
    raiseCurrentView();
}
void MainWindow::slotShowDeadLine( bool dead) {
    m_deadline = dead;
    currentView()->setShowDeadline( dead );
}
void MainWindow::slotShowCompleted( bool show) {
    m_completed = show;
    currentView()->setShowCompleted( m_completed );
}
void MainWindow::slotShowQuickTask( bool show ) {
    m_quicktask = show;
    if ( m_quicktask )
        m_curQuick->widget()->show();
    else
        m_curQuick->widget()->hide();
}
bool MainWindow::showOverDue()const {
    return m_overdue;
}
void MainWindow::setDocument( const QString& fi) {
    DocLnk doc(fi);
    if (doc.isValid() )
        receiveFile(doc.file() );
    else
        receiveFile(fi );
}

static const char *beamfile = "/tmp/opie-todo.vcs";
void MainWindow::slotBeam() {
    beam( currentView()->current() );
}
void MainWindow::beamDone( Ir* ir) {
    delete ir;
    ::unlink( beamfile );
}
void MainWindow::receiveFile( const QString& filename ) {
    OTodoAccessVCal* cal = new OTodoAccessVCal(filename );

    OTodoAccess acc( cal );
    acc.load();
    OTodoAccess::List list = acc.allRecords();

    if (list.count()){

	    QString message = QWidget::tr("<P>%1 new tasks arrived.<p>Would you like to add them to your Todolist?").arg(list.count() );

	    if ( QMessageBox::information(this, QWidget::tr("New Tasks"),
					  message, QMessageBox::Ok,
					  QMessageBox::Cancel ) == QMessageBox::Ok ) {
		    OTodoAccess::List::Iterator it;
		    for ( it = list.begin(); it != list.end(); ++it )
			    m_todoMgr.add( (*it) );

		    currentView()->updateView();
	    }
    }
}

void MainWindow::slotFlush() {
    m_syncing = TRUE;
    m_todoMgr.save();
}
void MainWindow::slotShowDetails() {
    slotShow( currentView()->current() );
}
/*
 * populate the Categories
 * Menu
 */
void MainWindow::populateCategories() {
    m_todoMgr.load();

    m_catMenu->clear();
    int id, rememberId;
    id = 1;
    rememberId = 1;

    m_catMenu->insertItem( QWidget::tr( "All Categories" ), id++ );
    m_catMenu->insertSeparator();
    QStringList categories = m_todoMgr.categories();
    categories.append( QWidget::tr( "Unfiled" ) );
    for ( QStringList::Iterator it = categories.begin();
	  it != categories.end(); ++it ) {
	m_catMenu->insertItem( *it, id );
	if ( *it == currentCategory() )
	    rememberId = id;
	++id;
    }
    setCategory( rememberId );
}
bool MainWindow::showCompleted()const {
    return m_completed;
}
bool MainWindow::showDeadline()const {
    return m_deadline;
}
bool MainWindow::showQuickTask()const {
    return m_quicktask;
}
QString MainWindow::currentCategory()const {
    return m_curCat;
}
int MainWindow::currentCatId() {
    return m_todoMgr.catId( m_curCat );
}
ViewBase* MainWindow::currentView() {
    return m_curView;
}
void MainWindow::raiseCurrentView() {
    // due QPE/Application/todolist show(int)
    // we might not have the populateCategories slot called once
    // we would show the otodo but then imediately switch to the currentView
    // if we're initially showing we shouldn't raise the table
    // in returnFromView we fix up m_showing
    if (m_showing ) return;

    m_stack->raiseWidget( m_curView->widget() );
}
void MainWindow::slotShowDue(bool ov) {
    m_overdue = ov;
    currentView()->showOverDue( ov );
    raiseCurrentView();
}
void MainWindow::slotShow( int uid ) {
    if ( uid == 0 ) return;
    qWarning("slotShow");
    currentShow()->slotShow( event( uid ) );
    m_stack->raiseWidget( currentShow()->widget() );
}
void MainWindow::slotShowNext() {
    int l = currentView()->next();
    if (l!=0)
        slotShow(l);
}
void MainWindow::slotShowPrev() {
    int l = currentView()->prev();
    if (l!=0)
        slotShow(l);
}
void MainWindow::slotEdit( int uid ) {
    if (uid == 0 ) return;
    if(m_syncing) {
	QMessageBox::warning(this, QWidget::tr("Todo"),
			     QWidget::tr("Data can't be edited, currently syncing"));
	return;
    }

    OTodo old_todo = m_todoMgr.event( uid );

    OTodo todo = currentEditor()->edit(this, old_todo );

    /* if completed */
    if ( currentEditor()->accepted() ) {
        handleAlarms( old_todo, todo );
        m_todoMgr.update( todo.uid(), todo );
	currentView()->replaceEvent( todo );
        /* a Category might have changed */
        populateCategories();
    }

    raiseCurrentView();
}
/*
void MainWindow::slotUpdate1( int uid, const SmallTodo& ev) {
    m_todoMgr.update( uid, ev );
}
*/
void MainWindow::updateTodo(  const OTodo& ev) {
    m_todoMgr.update( ev.uid() , ev );
}
/* The view changed it's configuration
 * update the view menu
 */
void MainWindow::slotUpdate3( QWidget* ) {

}
void MainWindow::updateList() {
    m_todoMgr.updateList();
}
void MainWindow::setReadAhead( uint count ) {
    if (m_todoMgr.todoDB() )
        m_todoMgr.todoDB()->setReadAhead( count );
}
void MainWindow::slotQuickEntered() {
    qWarning("entered");
    OTodo todo = quickEditor()->todo();
    if (todo.isEmpty() )
        return;

    m_todoMgr.add( todo );
    currentView()->addEvent( todo );
    raiseCurrentView();
}
QuickEditBase* MainWindow::quickEditor() {
    return m_curQuick;
}
void MainWindow::slotComplete( int uid ) {
    slotComplete( event(uid) );
}
void MainWindow::slotComplete( const OTodo& todo ) {
    OTodo to = todo;
    to.setCompleted( !to.isCompleted() );
    to.setCompletedDate( QDate::currentDate() );

    /*
     * if the item does recur
     * we need to spin it off
     * and update the items duedate to the next
     * possible recurrance of this item...
     * the spinned off one will loose the
     * recurrence.
     * We calculate the difference between the old due date and the
     * new one and add this diff to start, completed and alarm dates
     * -zecke
     */
    if ( to.hasRecurrence() && to.isCompleted() ) {
        OTodo to2( to );

        /* the spinned off one won't recur anymore */
        to.setRecurrence( ORecur() );

        ORecur rec = to2.recurrence();
        rec.setStart( to.dueDate() );
        to2.setRecurrence( rec );
        /*
         * if there is a next occurence
         * from the duedate of the last recurrance
         */
        QDate date;
        if ( to2.recurrence().nextOcurrence( to2.dueDate().addDays(1), date ) ) {
            int dayDiff = to.dueDate().daysTo( date );
            qWarning("day diff is %d", dayDiff );
            QDate inval;
            /* generate a new uid for the old record */
            to.setUid( 1 );

            /* add the old one cause it has a new UID here cause it was spin off */
            m_todoMgr.add( to );

            /*
             * update the due date
             * start date
             * and complete date
             */
            to2.setDueDate( date );
            rec.setStart( date );
            to2.setRecurrence( rec );  // could be Monday, TuesDay, Thursday every week

            /* move start date */
            if (to2.hasStartDate() )
                to2.setStartDate( to2.startDate().addDays( dayDiff ) );

            /* now the alarms */
            if (to2.hasNotifiers() ) {
                OPimNotifyManager::Alarms _als = to2.notifiers().alarms();
                OPimNotifyManager::Alarms als;

                /* for every alarm move the day */
                for ( OPimNotifyManager::Alarms::Iterator it = _als.begin(); it != _als.end(); ++it ) {
                    OPimAlarm al = (*it);
                    al.setDateTime( al.dateTime().addDays( dayDiff ) );
                    als.append( al );
                }
                to2.notifiers().setAlarms( als );
                handleAlarms( OTodo(), todo );
            }
            to2.setCompletedDate( inval );
            to2.setCompleted( false );

            updateTodo( to2 );
        }else
            updateTodo( to );
    }else
        updateTodo( to );

    currentView()->updateView();
    raiseCurrentView();
}
void MainWindow::flush() {
    slotFlush();
}
void MainWindow::reload() {
    slotReload();
}
int MainWindow::create() {
    int uid = 0;
    if(m_syncing) {
	QMessageBox::warning(this, QWidget::tr("Todo"),
			     QWidget::tr("Data can not be edited, currently syncing"));
	return uid;
    }
    m_todoMgr.load();


    OTodo todo = currentEditor()->newTodo( currentCatId(),
                                           this );

    if ( currentEditor()->accepted() ) {
	//todo.assignUid();
        uid = todo.uid();
        handleAlarms( OTodo(), todo );
        m_todoMgr.add( todo );
        currentView()->addEvent( todo );


        // I'm afraid we must call this every time now, otherwise
        // spend expensive time comparing all these strings...
        // but only call if we changed something -zecke
        populateCategories();
    }
    raiseCurrentView( );

    return uid;
}
/* delete it silently... */
bool MainWindow::remove( int uid ) {
    if (m_syncing) return false;

    /* argh need to get the whole OEvent... to disable alarms -zecke */
    handleAlarms( OTodo(), m_todoMgr.event( uid ) );

    return m_todoMgr.remove( uid );
}
void MainWindow::beam( int uid) {
    if( uid == 0 ) return;

    ::unlink( beamfile );
    m_todoMgr.load();

    OTodo todo = event( uid );
    OTodoAccessVCal* cal = new OTodoAccessVCal(QString::fromLatin1(beamfile) );
    OTodoAccess acc( cal );
    acc.load();
    acc.add( todo );
    acc.save();
    Ir* ir = new Ir(this );
    connect(ir, SIGNAL(done(Ir*) ),
            this, SLOT(beamDone(Ir*) ) );
    ir->send(beamfile, todo.summary(), "text/x-vCalendar" );
}
void MainWindow::show( int uid ) {
    m_todoMgr.load(); // might not be loaded yet
    m_showing = true;
    slotShow( uid );
    raise();
    QPEApplication::setKeepRunning();
}
void MainWindow::edit( int uid ) {
    m_todoMgr.load();
    slotEdit( uid );
}
void MainWindow::add( const OPimRecord& rec) {
    if ( rec.rtti() != OTodo::rtti() ) return;
    m_todoMgr.load(); // might not be loaded

    const OTodo& todo = static_cast<const OTodo&>(rec);

    m_todoMgr.add(todo );
    currentView()->addEvent( todo );


    // I'm afraid we must call this every time now, otherwise
    // spend expensive time comparing all these strings...
    // but only call if we changed something -zecke
    populateCategories();
}
void MainWindow::slotReturnFromView() {
    m_showing = false;
    raiseCurrentView();
}

namespace {
    OPimNotifyManager::Alarms findNonMatching( const OPimNotifyManager::Alarms& oldAls,
                                               const OPimNotifyManager::Alarms& newAls ) {
        OPimNotifyManager::Alarms nonMatching;
        OPimNotifyManager::Alarms::ConstIterator oldIt = oldAls.begin();
        OPimNotifyManager::Alarms::ConstIterator newIt;
        for ( ; oldIt != oldAls.end(); ++oldIt ) {
            bool found = false;
            QDateTime oldDt = (*oldIt).dateTime();
            for (newIt= newAls.begin(); newIt != newAls.end(); ++newIt ) {
                if ( oldDt == (*newIt).dateTime() ) {
                    found = true;
                    break;
                    }
            }
            if (!found)
                nonMatching.append( (*oldIt) );
        }
        return nonMatching;
    }
    void addAlarms( const OPimNotifyManager::Alarms& als, int uid ) {
        OPimNotifyManager::Alarms::ConstIterator it;
        for ( it = als.begin(); it != als.end(); ++it ) {
            qWarning("Adding alarm for %s", (*it).dateTime().toString().latin1() );
            AlarmServer::addAlarm( (*it).dateTime(), "QPE/Application/todolist", "alarm(QDateTime,int)", uid );
        }

    }
    void removeAlarms( const OPimNotifyManager::Alarms& als, int uid ) {
        OPimNotifyManager::Alarms::ConstIterator it;
        for ( it = als.begin(); it != als.end(); ++it ) {
            qWarning("Removinf alarm for %s",  (*it).dateTime().toString().latin1() );
            AlarmServer::deleteAlarm( (*it).dateTime(), "QPE/Application/todolist", "alarm(QDateTime,int)", uid );
        }
    }
}

void MainWindow::handleAlarms( const OTodo& oldTodo, const OTodo& newTodo) {
    /*
     * if oldTodo is not empty and has notifiers we need to find the deleted ones
     */
    if(!oldTodo.isEmpty() && oldTodo.hasNotifiers() ) {
        OPimNotifyManager::Alarms removed;
        OPimNotifyManager::Alarms oldAls = oldTodo.notifiers().alarms();
        if (!newTodo.hasNotifiers() )
            removed = oldAls;
        else
            removed = findNonMatching( oldAls, newTodo.notifiers().alarms() );

        removeAlarms( removed, oldTodo.uid() );
    }
    if ( newTodo.hasNotifiers() ) {
        OPimNotifyManager::Alarms added;
        if ( oldTodo.isEmpty() || !oldTodo.hasNotifiers() )
            added = newTodo.notifiers().alarms();
        else
            added = findNonMatching( newTodo.notifiers().alarms(), oldTodo.notifiers().alarms() );

        addAlarms( added, newTodo.uid() );
    }
}
/* we might have not loaded the db */
void MainWindow::doAlarm( const QDateTime& dt, int uid ) {
    m_todoMgr.load();

    OTodo todo = m_todoMgr.event( uid );
    if (!todo.hasNotifiers() ) return;

    /*
     * let's find the right alarm and find out if silent
     * then show a richtext widget
     */
    bool loud = false;
    OPimNotifyManager::Alarms als = todo.notifiers().alarms();
    OPimNotifyManager::Alarms::Iterator it;
    for ( it = als.begin(); it != als.end(); ++it ) {
        if ( (*it).dateTime() == dt ) {
            loud = ( (*it).sound() == OPimAlarm::Loud );
            break;
        }
    }
    if (loud)
        startAlarm();

    QDialog dlg(this, 0, TRUE );
    QVBoxLayout* lay = new QVBoxLayout( &dlg );
    QTextView* view = new QTextView( &dlg );
    lay->addWidget( view );
    QPushButton* btnOk = new QPushButton( tr("Ok"), &dlg );
    connect( btnOk, SIGNAL(clicked() ), &dlg, SLOT(accept() ) );
    lay->addWidget( btnOk );

    QString text = tr("<h1>Alarm at %1</h1><br>").arg( TimeString::dateString( dt ) );
    text += todo.toRichText();
    view->setText( text );

    dlg.showMaximized();
    bool needToStay = dlg.exec();

    if (loud)
        killAlarm();

    if (needToStay) {
//        showMaximized();
//        raise();
        QPEApplication::setKeepRunning();
//        setActiveWindow();
    }

}

