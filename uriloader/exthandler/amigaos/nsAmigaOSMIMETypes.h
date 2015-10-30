/* ***** BEGIN LICENSE BLOCK *****
 *
 * The contents of this file is copyrighted by Thomas and Hans-Joerg Frieden.
 * It's content is not open source and may not be redistributed, modified or adapted
 * without permission of the above-mentioned copyright holders.
 *
 * Since this code was originally developed under an AmigaOS related bounty, any derived
 * version of this file may only be used on an official AmigaOS system.
 *
 * Contributor(s):
 * 	Thomas Frieden <thomas@friedenhq.org>
 * 	Hans-Joerg Frieden <hans-joerg@friedenhq.org>
 *
 * ***** END LICENSE BLOCK ***** */

/* Automatically generated file */

struct AOSMimeType {
	const char *type;
	const char *extensions;
	const char *description;
	const char *handler;
	const char *handlerDesc;
	const char *iconFile;
};

struct AOSMimeType mimeTypes[] = {

    {
        "text/plain",
        "asc txt text pot brf",
        "Plain Text",
        "sys:utilities/MultiView",
        "MultiView",
        "def_txt.info"
     },
    {
        "text/html",
        "html htm shtml",
        "HTML Text",
        "timberwolf",
        "Web Browser",
        "def_html.info"
     },
    {
        "audio/basic",
        "au snd",
        "Audio Data",
        "sys:utilities/MultiView",
        "MultiView",
        "def_audio.info"
     },
    {
        "audio/x-wav",
        "wav",
        "RIFF WAV Audio Data",
        "sys:utilities/MultiView",
        "MultiView",
        "def_waw.info"
     },
    {
        "audio/x-aiff",
        "aif aiff aifc",
        "AIFF Audio",
        "sys:utilities/MultiView",
        "MultiView",
        "def_aiff.info"
     },
    {
        "application/pdf",
        "pdf",
        "Portable Document Format",
        "sys:utilities/AmiPDF/AmiPDF",
        "AmiPDF Viewer",
        "def_pdf.info"
     },
    {
        "application/x-pdf",
        "pdf",
        "Portable Document Format",
        "sys:utilities/AmiPDF/AmiPDF",
        "AmiPDF Viewer",
        "def_pdf.info"
     },
    {
        "application/x-bzpdf",
        "",
        "Portable Document Format",
        "sys:utilities/AmiPDF/AmiPDF",
        "AmiPDF Viewer",
        "def_pdf.info"
     },
    {
        "application/x-gzpdf",
        "",
        "Portable Document Format",
        "sys:utilities/AmiPDF/AmiPDF",
        "AmiPDF Viewer",
        "def_pdf.info"
     },
    {
        "application/postscript",
        "ps ai eps espi epsf eps2 eps3",
        "PostScript",
        "sys:utilities/AmiGS/AmiGS",
        "Amiga GhostScript",
        "def_ps.info"
     },
    {
        "application/x-bzpostscript",
        "",
        "PostScript",
        "sys:utilities/AmiGS/AmiGS",
        "Amiga GhostScript",
        "def_ps.info"
     },
    {
        "application/x-gzpostscript",
        "",
        "PostScript",
        "sys:utilities/AmiGS/AmiGS",
        "Amiga GhostScript",
        "def_ps.info"
     },
    {
        "image/x-eps",
        "",
        "PostScript",
        "sys:utilities/AmiGS/AmiGS",
        "Amiga GhostScript",
        "def_ps.info"
        ""
     },
    {
        "image/x-bzeps",
        "",
        "PostScript",
        "sys:utilities/AmiGS/AmiGS",
        "Amiga GhostScript",
        "def_ps.info"
     },
    {
        "image/x-gzeps",
        "",
        "PostScript",
        "sys:utilities/AmiGS/AmiGS",
        "Amiga GhostScript",
        "def_ps.info"
     },
    {
        "application/x-dvi",
        "dvi",
        "DVI Document format",
        "",
        "",
        "def_dvi.info"
     },
    {
        "application/x-gzdvi",
        "",
        "DVI Document format",
        "",
        "",
        "def_dvi.info"
     },
    {
        "application/x-bzdvi",
        "",
        "DVI Document format",
        "",
        "",
        "def_dvi.info"
     },
    {
        "image/tiff",
        "tiff tif",
        "TIFF Image",
        "sys:utilities/MultiView",
        "MultiView",
        "def_tiff.info"
     },

    {
        "application/x-7z-compressed",
        "7z",
        "7zip Archive",
        "sys:utilities/UnArc",
        "UnArc Utility",
        "def_7z.info"
     },

    {
        "application/x-bzip",
        "bz2",
        "BZip2 Compressed Data",
        "sys:utilities/UnArc",
        "UnArc Utility",
        "def_bz2.info"
     },

    {
        "application/x-gzip",
        "gz",
        "GZip Compressed Data",
        "sys:utilities/UnArc",
        "UnArc Utility",
        "def_gz.info"
     },

    {
        "application/x-lha",
        "lha",
        "LHa Archive",
        "sys:utilities/UnArc",
        "UnArc Utility",
        "def_lha.info"
     },
    {
        "application/x-lhz",
        "LZh",
        "LZa Archive",
        "sys:utilities/UnArc",
        "UnArc Utility",
        "def_lzh.info"
     },
    {
        "application/x-lzma",
        "",
        "lzma Compressed Data",
        "sys:utilities/UnArc",
        "UnArc Utility",
        "def_lzma.info"
     },

    {
        "application/x-rar",
        "rar",
        "RAR Archvive",
        "sys:utilities/UnArc",
        "UnArc Utility",
        "def_rar.info"
     },

    {
        "application/x-tar",
        "",
        "Tape Archive",
        "sys:utilities/UnArc",
        "UnArc Utility",
        "def_tar.info"
     },

    {
        "application/x-zip",
        "",
        "ZIP Archive",
        "sys:utilities/UnArc",
        "UnArc Utility",
        "def_zip.info"
     },

    {
        "application/zip",
        "zip",
        "ZIP Archive",
        "sys:utilities/UnArc",
        "UnArc Utility",
        "def_zip.info"
     },
    {
        "application/x-ogg",
        "",
        "Ogg Vorbis audio",
        "sys:utilities/MultiView",
        "MultiView",
        "def_ogg.info"
     },
    {
        "application/ogg",
        "ogx",
        "Ogg Media",
        "sys:utilities/MultiView",
        "MultiView",
        "def_ogg.info"
     },
    {
        "audio/x-mp3",
        "",
        "MP3 audio",
        "sys:utilities/MultiView",
        "MultiView",
        "def_mp3.info"
     },

    {
        "audio/x-mpeg",
        "",
        "MP3 audio",
        "sys:utilities/MultiView",
        "MultiView",
        "def_mp3.info"
     },
    {
        "audio/mpeg",
        "mpga mpega mp2 mp3 m4a",
        "MP3 audio",
        "sys:utilities/MultiView",
        "MultiView",
        "def_mp3.info"
     },

    {
        "application/x-tar",
        "tar",
        "Tape Archive",
        "sys:utilities/UnArc",
        "UnArc Utility",
        "def_tar.info"
     },
    {
        "text/plain",
        "asc txt text pot brf",
        "Plain Text",
        "sys:utilities/MultiView",
        "MultiView",
        "def_txt.info"
     },

    {
        "application/rtf",
        "rtf",
        "Rich Text Format",
        "sys:utilities/MultiView",
        "MultiView",
        "def_rtf.info"
     },

    {
        "image/x-ms-bmp",
        "bmp",
        "Windows BMP",
        "sys:utilities/MultiView",
        "MultiView",
        "def_bmp.info"
     },

    {
        "image/gif",
        "gif",
        "Graphics Interchange Format",
        "sys:utilities/MultiView",
        "MultiView",
        "def_gif.info"
     },
    {
        "image/jpeg",
        "jpeg jpg jpe",
        "Joint Photographic Expert Group image format",
        "sys:utilities/MultiView",
        "MultiView",
        "def_jpg.info"
     },
    {
        "image/png",
        "png",
        "Portable Network Graphics",
        "sys:utilities/MultiView",
        "MultiView",
        "def_png.info"
     },

    {
        "image/targa",
        "",
        "Targa Image Format",
        "sys:utilities/MultiView",
        "MultiView",
        "def_tga.info"
     },
    {
        "image/tiff",
        "tiff tif",
        "Tagged Image File Format",
        "sys:utilities/MultiView",
        "MultiView",
        "def_tiff.info"
     },

     {
         "video/3gpp",
         "3gp",
         "3GP Video File",
        "sys:utilities/MultiView",
        "MultiView",
         "def_video.info"
      },
  
     {
         "video/dv",
         "dif dv",
         "DV Digital Video",
        "sys:utilities/MultiView",
        "MultiView",
         "def_video.info"
      },

     {
         "application/xml",
         "xml xsl xsd",
         "XML Document",
         "sys:utilities/NotePad",
         "NotePad",
         "def_txt.info"
      },

     {
         "application/x-tex-pk",
         "pk",
         "TeX Packed Raster Font",
         "",
         "",
         "def_font.info"
      },
 
     {
         "application/x-redhat-package-manager",
         "rpm",
         "RedHead Package Manager archive",
         "Sys:Utilities/UNarc",
         "UnArc Utility",
         "def_archive.info"
      },
     {
         "text/x-vcard",
         "vcf",
         "VCARD address Data",
        "sys:utilities/MultiView",
        "MultiView",
		"def_bin.info"
      },
     {
         "text/x-c++src",
         "c++ cpp cxx cc",
         "C++ source code file",
        "sys:utilities/MultiView",
        "MultiView",
		"def_txt.info"
      },
     {
         "text/x-makefile",
         "",
         "Makefile",
        "sys:utilities/MultiView",
        "MultiView",
        "def_txt.info"
      },
     {
         "text/css",
         "css",
         "Cascading Style Sheet",
        "sys:utilities/MultiView",
        "MultiView",
        "def_txt.info"
      },
     {
         "application/java-vm",
         "class",
         "Java Virtual Machine File",
         "",
         "",
         "def_bin.info"
      },

     {
         "application/xhtml+xml",
         "xhtml xht",
         "XHTML Document",
        "sys:utilities/MultiView",
        "MultiView",
        "def_html.info"
      },
 
     {
         "application/x-msi",
         "msi",
         "Microsoft Installer for Window",
         "",
         "",
         "def_bin.info"
      },
 
     {
         "application/x-httpd-php4",
         "php4",
         "PHP Script source code",
         "Sys:Utilities/NotePad",
         "NotePad",
         "def_txt.info"
      },
     {
         "application/x-httpd-php3",
         "php3",
         "PHP Script source code",
        "sys:utilities/MultiView",
        "MultiView",
        "def_txt.info"
      },
  
     {
         "audio/x-pn-realaudio",
         "ra rm ram",
         "RealAudio stream",
         "",
         "",
         "def_sound.info"
      },
 
     {
         "application/x-msdos-program",
         "com exe bat dll",
         "MS/DOS program",
         "",
         "",
         "def_bin.info"
      },
     {
         "message/rfc822",
         "eml",
         "RFC822-formatted EMail",
        "sys:utilities/MultiView",
        "MultiView",
        "def_txt.info"
      },
 
     {
         "audio/midi",
         "mid midi kar",
         "MIDI Music file",
         "",
         "",
         "def_sound.info"
      },
     {
         "text/english",
         "",
         "English language text",
        "sys:utilities/MultiView",
        "MultiView",
        "def_txt.info"

      },
 
     {
         "application/x-ruby",
         "rb",
         "Ruby Script file",
        "sys:utilities/MultiView",
        "MultiView",
        "def_txt.info"
      },
     {
         "video/x-ms-wmx",
         "wmx",
         "Windows Media Redirector File",
        "sys:utilities/MultiView",
        "MultiView",
        "def_txt.info"
      },

     {
         "audio/x-ms-wma",
         "wma",
         "Windows Media Audio file",
        "sys:utilities/MultiView",
        "MultiView",
        "def_audio.info"
      },
 
     {
         "application/rss+xml",
         "rss",
         "RSS Feed",
        "sys:utilities/MultiView",
        "MultiView",
        "def_txt.info"
      },

     {
         "application/java-archive",
         "jar",
         "Java Archive",
         "Sys:Utilities/UnArc",
         "UnArc Utility",
         "def_archive.info"
      },
 
     {
         "application/x-abiword",
         "abw",
         "Abiword document file",
         "",
         "",
         "def_txt.info"
      },
 
     {
         "image/x-photoshop",
         "psd",
         "Photoshop image file",
        "sys:utilities/MultiView",
        "MultiView",
        "def_picture.info"
      },
 
     {
         "video/x-flv",
         "flv",
         "FLASH video file",
        "sys:utilities/MultiView",
        "MultiView",
        "def_video.info"
      },
     {
         "video/fli",
         "fli",
         "FLI Video file",
        "sys:utilities/MultiView",
        "MultiView",
        "def_video.info"
      },

     {
         "application/x-iso9660-image",
         "iso",
         "ISO9660 compatible ISO Image",
         "",
         "",
         "def_cd.info"
      },
  
     {
         "text/x-bibtex",
         "bib",
         "BiBTeX bibliography file",
        "sys:utilities/MultiView",
        "MultiView",
        "def_txt.info"
      },

     {
         "application/x-latex",
         "latex",
         "LaTeX source file",
        "sys:utilities/MultiView",
        "MultiView",
        "def_txt.info"
      },
 
     {
         "audio/x-realaudio",
         "ra",
         "RealAudio stream",
         "",
         "",
         "def_audio.info"
      },
     {
         "application/x-font",
         "pfa pfb gsf pcf pcf.Z",
         "PostScript font",
         "",
         "",
         "def_font.info"
      },
 
     {
         "audio/ogg",
         "oga ogg spx",
         "Ogg Vorbis audio stream",
        "sys:utilities/MultiView",
        "MultiView",
        "def_audio.info"
      },
     {
         "application/x-apple-diskimage",
         "dmg",
         "Apple Disk image",
         "",
         "",
         "def_bin.info"
      },
 
     {
         "application/javascript",
         "js",
         "JavaScript file",
         "timberwolf",
         "Web Browser",
         "def_js.info"
      },
     
     {
         "application/x-lzh",
         "lzh",
         "LZH Archive",
         "Sys:Utilities/UnArc",
         "UnArc Utility",
         "def_archive.info"
      },
 
     {
         "application/x-lzx",
         "lzx",
         "LZX Archive",
         "Sys:Utilities/UnArc",
         "UnArc Utility",
         "def_lzx.info"
      },
 
     {
         "video/x-ms-wmv",
         "wmv",
         "Windows Media Video stream",
		"sys:utilities/MultiView",
        "MultiView",
         "def_wmv.info"
      },
  
     {
         "video/mpeg",
         "mpeg mpg mpe",
         "MPEG video",
        "sys:utilities/MultiView",
        "MultiView",
        "def_mpeg.info"

      },
 
     {
         "text/x-java",
         "java",
         "JAVA source code",
        "sys:utilities/MultiView",
        "MultiView",
        "def_txt.info"
      },
  
     {
         "audio/flac",
         "flac",
         "FLAC Audio Codec",
        "sys:utilities/MultiView",
        "MultiView",
        "def_audio.info"
      },
 
     {
         "text/x-c++hdr",
         "h++ hpp hxx hh",
         "C++ header file",
        "sys:utilities/MultiView",
        "MultiView",
        "def_txt.info"
      },
 
     {
         "application/x-shockwave-flash",
         "swf swfl",
         "Shockwave Flash application",
         "",
         "",
         "def_bin.info"
      },
 
     {
         "application/vnd.mozilla.xul+xml",
         "xul",
         "XUL Application",
         "timberwolf",
         "Web Browser",
         "def_bin.info"
      },
 
     {
         "video/mp4",
         "mp4",
         "MPEG 4 Viddeo",
        "sys:utilities/MultiView",
        "MultiView",
        "def_mpeg.info"
      },
    
     {
         "video/x-msvideo",
         "avi",
         "Audio Video Interleaved stream",
        "sys:utilities/MultiView",
        "MultiView",
        "def_avi.info"
      },
 
     {
         "video/quicktime",
         "qt mov",
         "QuickTime movie",
        "sys:utilities/MultiView",
        "MultiView",
        "def_video.info"
      },
  
     {
         "application/octet-stream",
         "bin",
         "Binary Data",
         "",
         "",
         "def_bin.info"
      },
  
 
     {
         "application/rar",
         "rar",
         "RAR Archive",
         "Sys:Utilities/UnArc",
         "UnArc Utility",
         "def_rar.info"

      },
     {
         "video/ogg",
         "ogv",
         "Ogg video Container",
        "sys:utilities/MultiView",
        "MultiView",
        "def_video.info"
      },
  
     {
         "video/x-mng",
         "mng",
         "MNG Video",
        "sys:utilities/MultiView",
        "MultiView",
        "def_video.info"      
	},
  
    /* Terminator */
	{ (char *)0, }
};

