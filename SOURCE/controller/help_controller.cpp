#include "help_controller.h"
#include "../lib/platform/keyboard.h"
#include "../lib/ui/keys.h"
#include "../view/frame.h"
#include "../view/help_view.h"

HelpController::HelpController( HelpView &view, Frame &frame, Keyboard &keyboard )
  : view_( view ), frame_( frame ), keyboard_( keyboard ) {}

void HelpController::run() {
  int top = 0;
  int max = HelpView::lineCount() - HelpView::Rows;
  if ( max < 0 ) max = 0;
  for ( ;; ) {
    view_.draw( top );
    frame_.flush();
    unsigned key = keyboard_.read();
    if ( !isExtendedKey( key ) ) return;
    switch ( keyScan( key ) ) {
      case ScanUp: top--; break;
      case ScanDown: top++; break;
      case ScanPgUp: top -= HelpView::Rows; break;
      case ScanPgDn: top += HelpView::Rows; break;
      case ScanHome: top = 0; break;
      case ScanEnd: top = max; break;
      default: return;
    }
    if ( top > max ) top = max;
    if ( top < 0 ) top = 0;
  }
}
