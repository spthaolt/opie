
#include "stdpopups.h"
#include "popuphelper.h"

using namespace OpieTooth;

PopupHelper::PopupHelper() {
    init();
}

PopupHelper::~PopupHelper() {
}

void PopupHelper::insert( int id, popupFactory fact ) {
    m_map.insert(id, fact );
}

QPopupMenu* PopupHelper::find( int id, const Services& ser, BTDeviceItem* item ) {
    //owarn << "find" << oendl; 
    FactoryMap::Iterator it = m_map.find(id );
    if ( it != m_map.end() ) {
        //owarn << "found" << oendl; 
        popupFactory fact = it.data();
        return (*fact)(ser, item);
    }
    return 0l;
}

void PopupHelper::init() {
    insert( 4353, newRfcComPopup );
    insert( 4354, newDunPopup );
    insert( 4355, newRfcComPopup );
    insert( 4357, newObexPushPopup );
    insert( 4358, newObexFtpPopup );
    insert( 4374, newPanPopup );
    insert( 4375, newPanPopup );
    insert( 4388, newHidPopup );
}
