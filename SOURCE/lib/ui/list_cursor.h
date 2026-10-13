/*
   The selection and the scroll position of a list with rows visible rows.
*/

#ifndef DOSLIB_UI_LIST_CURSOR_H
#define DOSLIB_UI_LIST_CURSOR_H

class ListCursor {
public:
  ListCursor();
  void setRows( int rows );
  void setCount( int count );
  int count() const { return count_; }
  int sel() const { return sel_; }
  int top() const { return top_; }
  int rows() const { return rows_; }
  bool isEmpty() const { return count_ == 0; }

  /* Sets the selection and the first visible row as they are. */
  void place( int sel, int top );
  /* Selects index (unchecked) and scrolls it into view. */
  void select( int index );
  /* Keeps the selection within the list, and in view. */
  void clamp();
  /* Scrolls the selection into view. */
  void scrollToSelection();

private:
  int count_, sel_, top_, rows_;
};

#endif
