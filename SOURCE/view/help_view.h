/*
   F1: the keys of every page in a table that scrolls.
*/

#ifndef BMAGIC_VIEW_HELP_VIEW_H
#define BMAGIC_VIEW_HELP_VIEW_H

class Frame;

class HelpView {
public:
  enum { Rows = 21 };

  explicit HelpView( Frame &frame );
  static int lineCount();
  void draw( int top );

private:
  Frame &frame_;
};

#endif
