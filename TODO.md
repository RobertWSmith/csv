# CSV Project TODO / Wishlist

List of open ideas / to do's and supporting notes

* Define support for data sources other than those available from `fopen`
  * `stdin`
* Define support for compression libraries
  * Callback interface?
* Define callback interface to allow type conversion for CSV Fields
  * Predefine common conversions?
    * i.e. `char*` to `float`/`double`, `int`/`long`, `struct tm`?
    * convert to `wchar*`, ICU library strings?
  * ODBC style API w/ enum & void*?
