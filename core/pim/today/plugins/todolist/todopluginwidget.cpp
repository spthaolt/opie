/*
 * todopluginwidget.cpp
 *
 * copyright   : (c) 2002, 2003 by Maximilian Rei�
 * email       : harlekin@handhelds.org
 *
 */
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "todopluginwidget.h"

#include <qpe/config.h>
#include <qpe/qcopenvelope_qws.h>

using namespace Opie::Ui;
using namespace Opie;

TodolistPluginWidget::TodolistPluginWidget( QWidget *parent,  const char* name )
    : QWidget( parent, name )
    , todoLabel( new OClickableLabel( this ) )
    , layoutTodo( new QVBoxLayout( this ) )
    , todo( new OPimTodoAccess() )
{
    if ( !todo->load() )
        odebug << "Failed to load todo access" << oendl;

    layoutTodo->setAutoAdd( true );

    connect( todoLabel, SIGNAL( clicked() ), this, SLOT( startTodolist() ) );

    readConfig();
    getTodo();
}

TodolistPluginWidget::~TodolistPluginWidget() {
    delete todo;
    delete todoLabel;
    delete layoutTodo;
}


void TodolistPluginWidget::readConfig() {
    Config cfg( "todaytodoplugin" );
    cfg.setGroup( "config" );
    m_maxLinesTask = cfg.readNumEntry( "maxlinestask", 5 );
    m_maxCharClip =  cfg.readNumEntry( "maxcharclip", 38 );
}

void TodolistPluginWidget:: refresh() {
    todo->reload();
    getTodo();
}

void TodolistPluginWidget::reinitialize() {
    readConfig();
    todo->reload();
    getTodo();
}

/**
 * Get the todos
 */
void TodolistPluginWidget::getTodo() {


    QString output;
    QString tmpout;
    int count = 0;
    int lines = 0;

    // get overdue todos first
    m_list = todo->sorted( true, OPimTodoAccess::Deadline, OPimTodoAccess::OnlyOverDue, 1);

    for ( m_it = m_list.begin(); m_it != m_list.end(); ++m_it ) {
        if (!(*m_it).isCompleted() && ( lines < m_maxLinesTask ) ) {
            QString desc = (*m_it).summary();
            if( desc.isEmpty() ) {
                desc = (*m_it).description();
            }
            int days = (*m_it).dueDate().daysTo( QDate::currentDate() );
            tmpout += "<font color=#e00000><b>["
                    +  QString("%1").arg((*m_it).priority() )
                    + "] " + desc.mid( 0, m_maxCharClip )
                    + QObject::tr(" (%1 days ago)").arg( days )
                    + "</b></font><br>";

            lines++ ;
        }
    }

    // get total number of still open todos
    m_list = todo->sorted( true, OPimTodoAccess::Priority, OPimTodoAccess::DoNotShowCompleted, 1);

    for ( m_it = m_list.begin(); m_it != m_list.end(); ++m_it ) {
        count +=1;
        // not the overdues, we allready got them, and not if we are
        // over the maxlines
        if ( !(*m_it).isOverdue() && ( lines < m_maxLinesTask ) ) {
            QString desc = (*m_it).summary();
            if( desc.isEmpty() ) {
                desc = (*m_it).description();
            }
            tmpout += "<b> ["
                    +  QString("%1").arg((*m_it).priority() )
                    + "] </b>" + desc.mid( 0, m_maxCharClip );
            if( (*m_it).hasDueDate() ) {
                QString text;
                int off = QDate::currentDate().daysTo( (*m_it).dueDate() );
                if( off == 0 )
                    text = tr("today");
                else if( off == 1 )
                    text = tr("1 day"); // "tomorrow" wouldn't fit
                else
                    text = tr("%1 days").arg(QString::number(off));
                tmpout += " (" + text + ")";
            }
            tmpout += "<br>";
            lines++;
        }
    }

    if ( count > 0 ) {
        if( count == 1 ) {
            output += QObject::tr( "There is <b> 1</b> active task:  <br>" );
        } else {
            output += QObject::tr( "There are <b> %1</b> active tasks: <br>" ).arg( count );
        }
        output += tmpout;
    } else {
        output = QObject::tr( "No active tasks" );
    }
    todoLabel->setText( output );
}

/**
 * start the todolist
 */
void TodolistPluginWidget::startTodolist() {
    QCopEnvelope e( "QPE/System", "execute(QString)" );
    e << QString( "todolist" );
}
