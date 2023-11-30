// Relative import to be able to reuse the C sources.
// See the comment in ../{projectName}}.podspec for more information.

#include "../../../../QRMatrix/Encoder/alphanumericencoder.c"
#include "../../../../QRMatrix/Encoder/kanjiencoder.c"
#include "../../../../QRMatrix/Encoder/numericencoder.c"
#include "../../../../QRMatrix/Polynomial/polynomial.c"

#include "../../../../QRMatrix/common.c"
#include "../../../../QRMatrix/qrmatrixboard.c"
#include "../../../../QRMatrix/qrmatrixencoder.c"
#include "../../../../QRMatrix/qrmatrixextramode.c"
#include "../../../../QRMatrix/qrmatrixsegment.c"

#include "../../../../String/latinstring.c"
#include "../../../../String/shiftjisstring.c"
#include "../../../../String/shiftjisstringmap.c"
#include "../../../../String/unicodepoint.c"
#include "../../../../String/utf8string.c"

// #include "../../../../DevTools/devtools.c"
