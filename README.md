# FormGenWidgets-Qt

A Qt5 widgets library for easily generating (nested) input forms.

Build-in base types include booleans, integers, floats, date/time,
color, text, urls. These base types can be combined via 3 main types
of composition: records (think C structs), choice (i.e. tagged union)
and lists (ordered) resp. bags (unordered).

While admittedly not very pretty, the generated forms are (IMHO) quite
functional and require little code.

For an example app showcasing most features compile the library with
```
cmake -DFORMGENWIDGETS_QT_BUILD_TESTAPP=On
```
