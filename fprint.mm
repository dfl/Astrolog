/*
** Astrolog (Version 7.80) File: fprint.mm
**
** Native macOS print support using NSPrintOperation.
** Renders the current chart to a PDF via Cairo, then presents
** the native macOS print panel which includes printer selection,
** page setup, and "Save as PDF".
*/

#ifdef FLTK
#ifdef __APPLE__

#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>

// Include cairo via the include path that CMake provides (include/cairo/)
// so we use <cairo.h> not <cairo/cairo.h> to avoid homebrew path issues.
#ifdef CAIRO
#include <cairo.h>
#include <cairo-pdf.h>
#endif

// Include Astrolog headers
#include "astrolog.h"

#ifdef CAIRO

// Present the native macOS print panel for the current chart.
// Returns true if printing completed, false if cancelled or failed.

flag FPrintChartNative(void)
{
  // Create a temporary PDF file
  NSString *tmpDir = NSTemporaryDirectory();
  NSString *tmpPath = [tmpDir stringByAppendingPathComponent:@"astrolog_print.pdf"];
  const char *szPath = [tmpPath fileSystemRepresentation];

  // Render chart to PDF via Cairo
  cairo_surface_t *surface =
      cairo_pdf_surface_create(szPath, gs.xWin, gs.yWin);

  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    cairo_surface_destroy(surface);
    return fFalse;
  }

  InitBackendCairo(surface);
  GBClearScreen(gi.kiOff);
  gi.fFile = fFalse;
  DrawChartX();
  EndBackendCairo();
  cairo_surface_destroy(surface);

  // Load the PDF via PDFKit
  NSURL *fileURL = [NSURL fileURLWithPath:tmpPath];
  PDFDocument *pdfDoc = [[PDFDocument alloc] initWithURL:fileURL];

  if (pdfDoc) {
    NSPrintInfo *printInfo = [NSPrintInfo sharedPrintInfo];
    [printInfo setHorizontalPagination:NSPrintingPaginationModeFit];
    [printInfo setVerticalPagination:NSPrintingPaginationModeFit];

    NSPrintOperation *op = [pdfDoc printOperationForPrintInfo:printInfo
                                                  scalingMode:kPDFPrintPageScaleToFit
                                                   autoRotate:YES];
    [op setShowsPrintPanel:YES];
    [op setShowsProgressPanel:YES];
    [op runOperation];

    [[NSFileManager defaultManager] removeItemAtPath:tmpPath error:nil];
    return fTrue;
  }

  // Fallback: use NSImage to load PDF and print
  NSImage *image = [[NSImage alloc] initWithContentsOfURL:fileURL];
  if (!image) {
    [[NSFileManager defaultManager] removeItemAtPath:tmpPath error:nil];
    return fFalse;
  }

  NSImageView *imageView = [[NSImageView alloc] initWithFrame:
      NSMakeRect(0, 0, gs.xWin, gs.yWin)];
  [imageView setImage:image];
  [imageView setImageScaling:NSImageScaleProportionallyUpOrDown];

  NSPrintInfo *printInfo = [NSPrintInfo sharedPrintInfo];
  [printInfo setHorizontalPagination:NSPrintingPaginationModeFit];
  [printInfo setVerticalPagination:NSPrintingPaginationModeFit];

  NSPrintOperation *op = [NSPrintOperation printOperationWithView:imageView
                                                        printInfo:printInfo];
  [op setShowsPrintPanel:YES];
  [op setShowsProgressPanel:YES];
  [op runOperation];

  [[NSFileManager defaultManager] removeItemAtPath:tmpPath error:nil];
  return fTrue;
}

#else // !CAIRO

flag FPrintChartNative(void)
{
  return fFalse;
}

#endif // CAIRO
#endif // __APPLE__
#endif // FLTK
