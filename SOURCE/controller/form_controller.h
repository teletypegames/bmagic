/*
   A form (see FormView): Tab / Enter / the arrows move between the fields, the editing
   keys change the value (LineInput), Enter on the last field or F2 accepts it, Esc
   cancels it. The caller checks the values, and runs the form again with an error when
   they are wrong.
*/

#ifndef BMAGIC_CONTROLLER_FORM_CONTROLLER_H
#define BMAGIC_CONTROLLER_FORM_CONTROLLER_H

#include "line_input.h"
#include "../view/form_view.h"

class Keyboard;

class FormController {
public:
  FormController( FormView &view, Keyboard &keyboard, LineInput &input );
  /* false: cancelled. error: shown under the form (NULL: none); field: where the cursor
     starts. */
  bool run( const char *title, FormField *fields, int count, const char *error, int field );

private:
  void setField( int field );

  FormView &view_;
  Keyboard &keyboard_;
  LineInput &input_;
  FormField *fields_;
  FormCursor cursor_;
};

#endif
