/*
   A form in a box: a label and an edit field for each value, the hint of the active
   field (or an error) under them.
*/

#ifndef BMAGIC_VIEW_FORM_VIEW_H
#define BMAGIC_VIEW_FORM_VIEW_H

class Frame;

struct FormField {
  const char *label;
  char *value;
  int max; /* the longest value */
  const char *hint;
};

struct FormCursor {
  int field;  /* the active field */
  int pos;    /* the cursor in its value */
  int scroll; /* the first character shown */
};

class FormView {
public:
  enum { FieldWidth = 49 };

  explicit FormView( Frame &frame );
  /* The shown width of a field. */
  static int width( const FormField &field );
  void draw( const char *title, const FormField *fields, int count, const FormCursor &cursor, const char *error );

private:
  Frame &frame_;
};

#endif
