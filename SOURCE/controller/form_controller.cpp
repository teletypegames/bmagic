#include <stddef.h>

#include "form_controller.h"
#include "../lib/platform/keyboard.h"
#include "../lib/ui/keys.h"

FormController::FormController( FormView &view, Keyboard &keyboard, LineInput &input )
  : view_( view ), keyboard_( keyboard ), input_( input ), fields_( NULL ) {}

void FormController::setField( int field ) {
  cursor_.field = field;
  input_.start( fields_[field].value, fields_[field].max, FormView::width( fields_[field] ) );
}

bool FormController::run( const char *title, FormField *fields, int count, const char *error, int field ) {
  fields_ = fields;
  setField( field );
  for ( ;; ) {
    cursor_.pos = input_.pos();
    cursor_.scroll = input_.scroll();
    view_.draw( title, fields, count, cursor_, error );
    unsigned key = keyboard_.read();
    unsigned char ch = keyAscii( key ), scan = keyScan( key );
    error = NULL;
    if ( ch == KeyEsc ) return false;
    if ( ch == KeyEnter ) {
      if ( cursor_.field == count - 1 ) return true;
      setField( cursor_.field + 1 );
    }
    else if ( ch == KeyTab ) setField( ( cursor_.field + 1 ) % count );
    else if ( isExtendedKey( key ) && scan == ScanF2 ) return true;
    else if ( isExtendedKey( key ) && ( scan == ScanUp || scan == ScanTab ) )
      setField( ( cursor_.field + count - 1 ) % count );
    else if ( isExtendedKey( key ) && scan == ScanDown ) setField( ( cursor_.field + 1 ) % count );
    else input_.handle( key );
  }
}
