/*
 * Header includes for ieframe.dll
 *
 * Copyright 2011 Jacek Caban for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdarg.h>

#define COBJMACROS

#include "windef.h"
#include "winbase.h"
#include "wingdi.h"
#include "winuser.h"

#include "ole2.h"
#include "olectl.h"
#include "shlobj.h"
#include "mshtmhst.h"
#include "exdisp.h"
#include "hlink.h"
#include "htiface.h"
#include "shdeprecated.h"
#include "docobjectservice.h"

#include "wine/list.h"

typedef struct ConnectionPoint ConnectionPoint;
typedef struct DocHost DocHost;

typedef struct {
    IConnectionPointContainer IConnectionPointContainer_iface;

    ConnectionPoint *wbe2;
    ConnectionPoint *wbe;
    ConnectionPoint *pns;

    IUnknown *impl;
} ConnectionPointContainer;

typedef struct {
    IHlinkFrame    IHlinkFrame_iface;
    ITargetFrame   ITargetFrame_iface;
    ITargetFrame2  ITargetFrame2_iface;
    ITargetFramePriv2 ITargetFramePriv2_iface;
    IWebBrowserPriv2IE9 IWebBrowserPriv2IE9_iface;

    IUnknown *outer;
    DocHost *doc_host;
} HlinkFrame;

struct _task_header_t;

typedef void (*task_proc_t)(DocHost*, struct _task_header_t*);
typedef void (*task_destr_t)(struct _task_header_t*);

typedef struct _task_header_t {
    struct list entry;
    task_proc_t proc;
    task_destr_t destr;
} task_header_t;

typedef struct {
    IShellBrowser IShellBrowser_iface;
    IBrowserService IBrowserService_iface;
    IDocObjectService IDocObjectService_iface;

    LONG ref;

    DocHost *doc_host;
}  ShellBrowser;

typedef struct {
    IHTMLWindow2 IHTMLWindow2_iface;
    DocHost *doc_host;
} IEHTMLWindow;

typedef struct {
    INewWindowManager INewWindowManager_iface;
    DocHost *doc_host;
} NewWindowManager;

typedef struct {
    WCHAR *url;
    IStream *stream;
} travellog_entry_t;

typedef struct _IDocHostContainerVtbl
{
    ULONG (*addref)(DocHost*);
    ULONG (*release)(DocHost*);
    void (*get_docobj_rect)(DocHost*,RECT*);
    HRESULT (*set_status_text)(DocHost*,const WCHAR*);
    void (*on_command_state_change)(DocHost*,LONG,BOOL);
    void (*set_url)(DocHost*,const WCHAR*);
} IDocHostContainerVtbl;

struct DocHost {
    IOleClientSite      IOleClientSite_iface;
    IOleInPlaceSiteEx   IOleInPlaceSiteEx_iface;
    IDocHostUIHandler2  IDocHostUIHandler2_iface;
    IOleDocumentSite    IOleDocumentSite_iface;
    IOleControlSite     IOleControlSite_iface;
    IOleCommandTarget   IOleCommandTarget_iface;
    IDispatch           IDispatch_iface;
    IPropertyNotifySink IPropertyNotifySink_iface;
    IServiceProvider    IServiceProvider_iface;

    /* Interfaces of InPlaceFrame object */
    IOleInPlaceFrame  IOleInPlaceFrame_iface;

    IWebBrowser2 *wb;

    IDispatch *client_disp;
    IDocHostUIHandler *hostui;
    IOleInPlaceFrame *frame;
    IOleCommandTarget *olecmd;

    IUnknown *document;
    IOleDocumentView *view;
    IUnknown *doc_navigate;

    const IDocHostContainerVtbl *container_vtbl;

    HWND hwnd;
    HWND frame_hwnd;

    struct list task_queue;

    LPOLESTR url;

    VARIANT_BOOL silent;
    VARIANT_BOOL offline;
    VARIANT_BOOL busy;

    READYSTATE ready_state;
    READYSTATE doc_state;
    DWORD prop_notif_cookie;
    BOOL is_prop_notif;

    ShellBrowser *browser_service;
    IShellUIHelper2 *shell_ui_helper;

    struct {
        travellog_entry_t *log;
        unsigned size;
        unsigned length;
        unsigned position;
        int loading_pos;
    } travellog;

    ConnectionPointContainer cps;
    IEHTMLWindow html_window;
    NewWindowManager nwm;
};

struct WebBrowser {
    IUnknown                 IUnknown_inner;
    IWebBrowser2             IWebBrowser2_iface;
    IOleObject               IOleObject_iface;
    IOleInPlaceObject        IOleInPlaceObject_iface;
    IOleControl              IOleControl_iface;
    IPersistStorage          IPersistStorage_iface;
    IPersistMemory           IPersistMemory_iface;
    IPersistStreamInit       IPersistStreamInit_iface;
    IProvideClassInfo2       IProvideClassInfo2_iface;
    IViewObject2             IViewObject2_iface;
    IOleInPlaceActiveObject  IOleInPlaceActiveObject_iface;
    IOleCommandTarget        IOleCommandTarget_iface;
    IServiceProvider         IServiceProvider_iface;
    IDataObject              IDataObject_iface;
    HlinkFrame hlink_frame;

    LONG ref;

    INT version;
    DWORD tid;

    IOleClientSite *client;
    IOleClientSite *client_closed;
    IOleContainer *container;
    IOleInPlaceSiteEx *inplace;

    IAdviseSink *sink;
    DWORD sink_aspects;
    DWORD sink_flags;

    IOleAdviseHolder *advise_holder;

    /* window context */

    HWND frame_hwnd;
    IOleInPlaceUIWindow *uiwindow;
    RECT pos_rect;
    RECT clip_rect;
    OLEINPLACEFRAMEINFO frameinfo;
    SIZEL extent;

    BOOL ui_activated;

    HWND shell_embedding_hwnd;

    VARIANT_BOOL register_browser;
    VARIANT_BOOL visible;
    VARIANT_BOOL menu_bar;
    VARIANT_BOOL address_bar;
    VARIANT_BOOL status_bar;
    VARIANT_BOOL tool_bar;
    VARIANT_BOOL full_screen;
    VARIANT_BOOL theater_mode;

    DocHost doc_host;
};

struct InternetExplorer {
    DocHost doc_host;
    IWebBrowser2 IWebBrowser2_iface;
    IExternalConnection IExternalConnection_iface;
    IServiceProvider IServiceProvider_iface;
    HlinkFrame hlink_frame;

    LONG ref;
    LONG extern_ref;

    HWND frame_hwnd;
    HWND status_hwnd;
    HWND toolbar_hwnd;
    HMENU menu;
    BOOL nohome;

    struct list entry;
};

void WebBrowser_OleObject_Init(WebBrowser*);
void WebBrowser_ViewObject_Init(WebBrowser*);
void WebBrowser_Persist_Init(WebBrowser*);
void WebBrowser_ClassInfo_Init(WebBrowser*);

void WebBrowser_OleObject_Destroy(WebBrowser*);

void DocHost_Init(DocHost*,IWebBrowser2*,const IDocHostContainerVtbl*);
void DocHost_Release(DocHost*);
void DocHost_ClientSite_Init(DocHost*);
void DocHost_ClientSite_Release(DocHost*);
void DocHost_Frame_Init(DocHost*);
void release_dochost_client(DocHost*);

void IEHTMLWindow_Init(DocHost*);
void NewWindowManager_Init(DocHost*);

void HlinkFrame_Init(HlinkFrame*,IUnknown*,DocHost*);
BOOL HlinkFrame_QI(HlinkFrame*,REFIID,void**);

HRESULT create_browser_service(DocHost*,ShellBrowser**);
void detach_browser_service(ShellBrowser*);
HRESULT create_shell_ui_helper(IShellUIHelper2**);

void ConnectionPointContainer_Init(ConnectionPointContainer*,IUnknown*);
void ConnectionPointContainer_Destroy(ConnectionPointContainer*);

void call_sink(ConnectionPoint*,DISPID,DISPPARAMS*);
HRESULT navigate_url(DocHost*,LPCWSTR,const VARIANT*,const VARIANT*,VARIANT*,VARIANT*);
HRESULT go_home(DocHost*);
HRESULT go_back(DocHost*);
HRESULT go_forward(DocHost*);
HRESULT refresh_document(DocHost*,const VARIANT*);
HRESULT get_window(DocHost*,IHTMLWindow2**);
HRESULT get_location_url(DocHost*,BSTR*);
HRESULT set_dochost_url(DocHost*,const WCHAR*);
void handle_navigation_error(DocHost*,HRESULT,BSTR,IHTMLWindow2*);
const WCHAR *error_url_frag(const WCHAR*);
HRESULT dochost_object_available(DocHost*,IUnknown*);
void set_doc_state(DocHost*,READYSTATE);
void activate_document(DocHost*);
void deactivate_document(DocHost*);
void create_doc_view_hwnd(DocHost*);
void on_commandstate_change(DocHost*,LONG,BOOL);
void notify_download_state(DocHost*,BOOL);
void update_navigation_commands(DocHost *dochost);

#define WM_DOCHOSTTASK (WM_USER+0x300)
#define WM_GETMARSHALEDDOC (WM_USER+0x400)
void push_dochost_task(DocHost*,task_header_t*,task_proc_t,task_destr_t,BOOL);
void abort_dochost_tasks(DocHost*,task_proc_t);
LRESULT process_dochost_tasks(DocHost*);

void InternetExplorer_WebBrowser_Init(InternetExplorer*);
HRESULT update_ie_statustext(InternetExplorer*, LPCWSTR);
void released_obj(void);
DWORD release_extern_ref(InternetExplorer*,BOOL);

void register_iewindow_class(void);
void unregister_iewindow_class(void);

#define TID_LIST \
    XCLSID(WebBrowser) \
    XCLSID(WebBrowser_V1) \
    XIID(IWebBrowser2)

typedef enum {
#define XIID(iface) iface ## _tid,
#define XCLSID(class) class ## _tid,
TID_LIST
#undef XIID
#undef XCLSID
    LAST_tid
} tid_t;

HRESULT get_typeinfo(tid_t,ITypeInfo**);

HRESULT WINAPI CUrlHistory_Create(IClassFactory*,IUnknown*,REFIID,void**);
HRESULT WINAPI InternetExplorer_Create(IClassFactory*,IUnknown*,REFIID,void**);
HRESULT WINAPI InternetShortcut_Create(IClassFactory*,IUnknown*,REFIID,void**);
HRESULT WINAPI WebBrowser_Create(IClassFactory*,IUnknown*,REFIID,void**);
HRESULT WINAPI WebBrowserV1_Create(IClassFactory*,IUnknown*,REFIID,void**);
HRESULT WINAPI InternetExplorerManager_Create(IClassFactory*,IUnknown*,REFIID,void**);

extern IClassFactory InternetExplorerFactory;
extern IClassFactory InternetExplorerManagerFactory;

extern LONG module_ref;
extern HINSTANCE ieframe_instance;

static inline void lock_module(void) {
    InterlockedIncrement(&module_ref);
}

static inline void unlock_module(void) {
    InterlockedDecrement(&module_ref);
}

static inline LPWSTR co_strdupW(LPCWSTR str)
{
    WCHAR *ret = CoTaskMemAlloc((lstrlenW(str) + 1)*sizeof(WCHAR));
    if (ret)
        lstrcpyW(ret, str);
    return ret;
}

static inline LPWSTR co_strdupAtoW(LPCSTR str)
{
    INT len;
    WCHAR *ret;
    len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    ret = CoTaskMemAlloc(len*sizeof(WCHAR));
    if (ret)
        MultiByteToWideChar(CP_ACP, 0, str, -1, ret, len);
    return ret;
}

static inline LPSTR co_strdupWtoA(LPCWSTR str)
{
    INT len;
    CHAR *ret;
    len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, 0, 0);
    ret = CoTaskMemAlloc(len);
    if (ret)
        WideCharToMultiByte(CP_ACP, 0, str, -1, ret, len, 0, 0);
    return ret;
}

enum SessionOp
{
    SESSION_QUERY,
    SESSION_INCREMENT,
    SESSION_DECREMENT
};

LONG WINAPI SetQueryNetSessionCount(DWORD session_op);
