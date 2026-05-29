// ============================================================================
// JpegStubs.cpp — Minimal stubs for libjpeg and turbojpeg
//
// The original code uses libjpeg API directly for screenshot saving and
// texture loading. These stubs allow the build to link. Real JPEG support
// will come from cross-compiled libjpeg-turbo or stb_image replacement.
// ============================================================================

#include <cstdio>
#include <cstring>

extern "C" {

// ---- libjpeg stubs ----

struct jpeg_error_mgr {
    int dummy;
};

struct jpeg_compress_struct {
    int dummy;
};

struct jpeg_decompress_struct {
    int dummy;
};

typedef struct jpeg_error_mgr* jpeg_error_ptr;

static jpeg_error_mgr g_stub_jerr;

void* jpeg_std_error(jpeg_error_mgr*) { return &g_stub_jerr; }
void jpeg_CreateCompress(void*, int, size_t) {}
void jpeg_stdio_dest(void*, FILE*) {}
void jpeg_set_defaults(void*) {}
void jpeg_set_quality(void*, int, int) {}
void jpeg_start_compress(void*, int) {}
void jpeg_write_scanlines(void*, void**, int) {}
void jpeg_finish_compress(void*) {}
void jpeg_destroy_compress(void*) {}

void jpeg_CreateDecompress(void*, int, size_t) {}
void jpeg_stdio_src(void*, FILE*) {}
int jpeg_read_header(void*, int) { return 0; }
int jpeg_start_decompress(void*) { return 0; }
void jpeg_read_scanlines(void*, void**, int) {}
int jpeg_finish_decompress(void*) { return 0; }
void jpeg_destroy_decompress(void*) {}

// ---- turbojpeg stubs ----

int tjInitDecompress() { return -1; }
int tjDecompressHeader3(void*, const unsigned char*, unsigned long, int*, int*, int*, int*) { return -1; }
int tjDestroy(int) { return 0; }
int tjDecompress2(void*, const unsigned char*, unsigned long, unsigned char*, int, int, int, int, int) { return -1; }

}
