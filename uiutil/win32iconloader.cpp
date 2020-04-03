// Copyright © 2020 Jonas Kümmerlin <jonas@kuemmerlin.eu>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "win32iconloader.h"

#ifdef WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <QPixmap>
#include <QtWin>
#include <QIcon>
#include <QTimer>
#include <QFile>
#include <vector>
#include <map>
#include <shellapi.h>

namespace {

#pragma pack(push, 1)
struct GrpIconDirEntry {
    BYTE   bWidth;
    BYTE   bHeight;
    BYTE   bColorCount;
    BYTE   bReserved;
    WORD   wPlanes;
    WORD   wBitCount;
    DWORD  dwBytesInRes;
    WORD   nID;
};
struct GrpIconDir {
    WORD idReserved;
    WORD idType;
    WORD idCount;
    GrpIconDirEntry idEntries[];
};
struct IconDirEntry {
    BYTE bWidth;
    BYTE bHeight;
    BYTE bColorCount;
    BYTE bReserved;
    WORD wPlanes;
    WORD wBitCount;
    DWORD dwBytesInRes;
    DWORD dwImageOffset;
};
struct IconDir {
    WORD idReserved;
    WORD idType;
    WORD idCount;
    IconDirEntry idEntries[];
};
#pragma pack(pop)

QPixmap iconResourceToImage(const void *resource, int bytesize)
{
    HICON icon;

    if (!memcmp(resource, "\211PNG\r\n\032\n", std::min(8, bytesize))) {
        // jumbo icon in PNG format - take a shortcut right here,
        // not just because its more efficient, but because CreateIconFromResource doesn't like it
        return QPixmap::fromImage(QImage::fromData((const uchar*)resource, bytesize, "PNG"));
    } else if ((icon = CreateIconFromResourceEx((BYTE*)resource, bytesize, TRUE, 0x00030000, 0, 0, 0))) {
        QPixmap p = QtWin::fromHICON(icon);
        DestroyIcon(icon);
        return p;
    } else {
        return QPixmap();
    }
}

QPixmap iconResourceToImage(HMODULE mod, LPWSTR id)
{
    HRSRC iconRes = FindResourceW(mod, id, RT_ICON);
    if (!iconRes)
        return QPixmap();

    HGLOBAL loadedIcon = LoadResource(mod, iconRes);
    if (!loadedIcon)
        return QPixmap();


    const void *iconBytes = LockResource(loadedIcon);
    if (!iconBytes)
        return QPixmap();

    int iconLength = SizeofResource(mod, iconRes);
    if (!iconLength)
        return QPixmap();

    return iconResourceToImage(iconBytes, iconLength);
}

struct ResourceNameFromIndex_closure {
    HRSRC retval;
    int index;
};

BOOL CALLBACK resourceNameFromIndex_Enumerator(HMODULE module, LPCWSTR type, LPWSTR name, LONG_PTR param) {
    auto closure = (struct ResourceNameFromIndex_closure *)param;

    closure->index--;
    if (closure->index < 0) {
        closure->retval = FindResourceW(module, name, type);
        return FALSE;
    }

    return TRUE;
}

HRSRC findResourceByIndex(HMODULE module, int index)
{
    struct ResourceNameFromIndex_closure param = { nullptr, index };

    EnumResourceNamesW(module, RT_GROUP_ICON, resourceNameFromIndex_Enumerator, (LONG_PTR)&param);

    return param.retval;
}

std::vector<QPixmap> loadIconImagesFromDll(const QString &dll, int index)
{
    std::vector<QPixmap> v;
    std::map<WORD, GrpIconDirEntry> bestIcons;
    GrpIconDir *grpIconDir;
    HGLOBAL loadedGrpIconDir;
    HRSRC res;

    // Load DLL
    HMODULE mod = LoadLibraryExW((LPCWSTR)dll.utf16(), NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (!mod)
        goto out;

    // find resource
    if (index < 0)
        res = FindResource(mod, MAKEINTRESOURCE(-index), RT_GROUP_ICON);
    else
        res = findResourceByIndex(mod, index);
    if (!res)
        goto out;

    // load and lock resource
    loadedGrpIconDir = LoadResource(mod, res);
    if (!loadedGrpIconDir)
        goto out;

    grpIconDir = (GrpIconDir*)LockResource(loadedGrpIconDir);

    // find highest-bpp icons
    for (WORD i = 0; i < grpIconDir->idCount; ++i) {
        // icons are available in multiple bpp values - we only want the highest one
        // since about everyone has 32bit RGBA display these days
        WORD wh = (grpIconDir->idEntries[i].bWidth << 8) | (grpIconDir->idEntries[i].bHeight);
        if (grpIconDir->idEntries[i].wBitCount >= bestIcons[wh].wBitCount)
            bestIcons[wh] = grpIconDir->idEntries[i];
    }

    // now load all the icons
    for (const auto &p : bestIcons) {
        QPixmap i = iconResourceToImage(mod, MAKEINTRESOURCEW(p.second.nID));
        if (!i.isNull())
            v.push_back(i);
    }

out:
    if (mod) FreeLibrary(mod);

    return v;
}

std::vector<QPixmap> loadIconImagesFromIco(const QString &file)
{
    std::vector<QPixmap> v;
    std::map<WORD, IconDirEntry> bestIcons;

    QFile f(file);
    if (!f.open(QFile::ReadOnly))
        return v;

    const uchar *data = f.map(0, f.size());
    IconDir *id = (IconDir *)data;

    for (WORD i = 0; i < id->idCount; ++i) {
        // icons are available in multiple bpp values - we only want the highest one
        // since about everyone has 32bit RGBA display these days
        WORD wh = (id->idEntries[i].bWidth << 8) | (id->idEntries[i].bHeight);
        if (id->idEntries[i].wBitCount >= bestIcons[wh].wBitCount)
            bestIcons[wh] = id->idEntries[i];
    }

    // now load all the icons
    for (const auto &p : bestIcons) {
        QPixmap i = iconResourceToImage(data + p.second.dwImageOffset, p.second.dwBytesInRes);
        if (!i.isNull())
            v.push_back(i);
    }

    return v;
}

std::vector<QPixmap> loadIconImages(const QString &file, int index)
{
    if (file.endsWith(QLatin1String(".ico"), Qt::CaseInsensitive)) {
        return loadIconImagesFromIco(file);
    } else {
        return loadIconImagesFromDll(file, index);
    }
}

} // anonymous namespace

QIcon IconLoader::fromFile(const QString &file, int index)
{
    QIcon i;
    auto v = loadIconImages(file, index);
    for (const QPixmap &p : v) {
        i.addPixmap(p);
    }
    return i;
}

QIcon IconLoader::fromShellStock(SHSTOCKICONID siid)
{
    SHSTOCKICONINFO sii;
    ZeroMemory(&sii, sizeof(sii));

    sii.cbSize = sizeof(sii);
    HRESULT hr = SHGetStockIconInfo(siid, SHGSI_ICONLOCATION, &sii);
    if (SUCCEEDED(hr)) {
        return IconLoader::fromFile(QString::fromWCharArray(sii.szPath), sii.iIcon);
    } else {
        return QIcon();
    }
}

#endif // WIN32
