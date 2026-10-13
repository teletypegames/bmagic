#ifndef DOSLIB_CORE_SORT_H
#define DOSLIB_CORE_SORT_H

/* Shell sort with Ciura's gaps: small, no recursion, fine for a few hundred items. */
template <class T> void shellSort( T *a, int n, int ( *cmp )( T, T ) ) {
  static const int gaps[] = { 701, 301, 132, 57, 23, 10, 4, 1, 0 };
  for ( const int *gap = gaps; *gap; gap++ ) {
    if ( *gap >= n ) continue;
    for ( int i = *gap; i < n; i++ ) {
      T key = a[i];
      int j = i;
      for ( ; j >= *gap && cmp( a[j - *gap], key ) > 0; j -= *gap ) a[j] = a[j - *gap];
      a[j] = key;
    }
  }
}

/* The same with a comparison object (cmp.compare( a, b )). */
template <class T, class C> void shellSortBy( T *a, int n, C &cmp ) {
  static const int gaps[] = { 701, 301, 132, 57, 23, 10, 4, 1, 0 };
  for ( const int *gap = gaps; *gap; gap++ ) {
    if ( *gap >= n ) continue;
    for ( int i = *gap; i < n; i++ ) {
      T key = a[i];
      int j = i;
      for ( ; j >= *gap && cmp.compare( a[j - *gap], key ) > 0; j -= *gap ) a[j] = a[j - *gap];
      a[j] = key;
    }
  }
}

#endif
