/*
   F1: the help scrolls with the arrows, PgUp / PgDn, Home / End; any other key closes it.
*/

#ifndef BMAGIC_CONTROLLER_HELP_CONTROLLER_H
#define BMAGIC_CONTROLLER_HELP_CONTROLLER_H

class Frame;
class HelpView;
class Keyboard;

class HelpController {
public:
  HelpController( HelpView &view, Frame &frame, Keyboard &keyboard );
  void run();

private:
  HelpView &view_;
  Frame &frame_;
  Keyboard &keyboard_;
};

#endif
