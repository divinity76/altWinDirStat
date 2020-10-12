// XySlider.cpp	- Implementation of CXySlider
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#include "stdafx.h"
#pragma once




#ifndef WDS_XYSLIDER_CPP
#define WDS_XYSLIDER_CPP

WDS_FILE_INCLUDE_MESSAGE


#include "datastructures.h"
#include "xyslider.h"
#include "globalhelpers.h"
#include "hwnd_funcs.h"
#include "macros_that_scare_small_children.h"
#include "ScopeGuard.h"

namespace {

	[[nodiscard]] std::wstring pts(const POINT& pt) {
		return L"pt.x: " + std::to_wstring(pt.x) + L", pt.y: " + std::to_wstring(pt.y);
		}

	[[nodiscard]] bool pts_eq(const POINT& ptl, const POINT& ptr) noexcept {
		if (ptl.x != ptr.x) {
			return false;
			}
		if (ptl.y != ptr.y) {
			return false;
			}
		// I could do this above, but I don't like undefined behavior.
		// Is POINT even packed?
		ASSERT(memcmp(&ptl, &ptr, sizeof(POINT)) == 0);
		return true;
		}

	constexpr const int GRIPPER_RADIUS = 8;

	void move_to_coord( _In_ const HDC hDC, _In_ const int rc_x, _In_ const int rc_y, _In_ const HDC hAttribDC ) {
		// https://docs.microsoft.com/en-us/cpp/mfc/reference/cdc-class?view=vs-2019#m_hdc
		// "By default, m_hDC is equal to m_hAttribDC, the other device context wrapped by CDC"
		ASSERT(hDC == hAttribDC);

		//pdc.MoveTo( rc.left,  m_zero.y ); <---Not handling the return value means that WE DO NOT care about the previous "current position", thus the fourth parameter to MoveToEx should be NULL.
		POINT pt = {}; //For debugging.
		TRACE(L"pt: %s\r\n", pts(pt).c_str());
		ASSERT( hDC != nullptr );
		POINT before_hDC = {};
		if ( hDC != hAttribDC ) {
			//If [MoveToEx] succeeds, the return value is nonzero. If [MoveToEx] fails, the return value is zero.
			VERIFY( ::MoveToEx( hDC, rc_x, rc_y, &pt ) );
			}
		POINT after_hDC = pt;
		ASSERT(pts_eq(before_hDC, after_hDC));
		TRACE(L"pt: %s\r\n", pts(pt).c_str());
		if ( hAttribDC != nullptr ) {
			//If [MoveToEx] succeeds, the return value is nonzero. If [MoveToEx] fails, the return value is zero.
			VERIFY( ::MoveToEx( hAttribDC, rc_x, rc_y, &pt) );
			}
		POINT after_hAttribDC = pt;
		TRACE(L"pt: %s\r\n", pts(pt).c_str());
		if (!pts_eq(after_hDC, after_hAttribDC)) {
			ASSERT(hDC == hAttribDC);
			TRACE(L"difference here matters!!!\r\n");
			TRACE(L"\r\n%s\r\n%s\r\n", pts(after_hDC).c_str(), pts(after_hAttribDC).c_str());
			}
		}
	
	void line_to_coord( _In_ const HDC hDC, _In_ const int rc_x, _In_ const int rc_y, _In_ const HDC hAttribDC ) {
		ASSERT( hDC != nullptr );
		// https://docs.microsoft.com/en-us/cpp/mfc/reference/cdc-class?view=vs-2019#m_hdc
		// "By default, m_hDC is equal to m_hAttribDC, the other device context wrapped by CDC"
		ASSERT(hDC == hAttribDC);
		if ( ( hAttribDC != nullptr ) && ( hDC != hAttribDC ) ) {
			//If [MoveToEx] succeeds, the return value is nonzero. If [MoveToEx] fails, the return value is zero.
			VERIFY( ::MoveToEx( hAttribDC, rc_x, rc_y, nullptr ) );
			}

		//If [LineTo] succeeds, the return value is nonzero. If [LineTo] fails, the return value is zero.
		VERIFY( ::LineTo( hDC, rc_x, rc_y ) );
		}


	void fill_solid_rectangle( _In_ const HDC m_hDC, _In_ const RECT rc, _In_ const COLORREF clr ) {
		//ASSERT( m_hDC != NULL );

		////If [SetBkColor] fails, the return value is CLR_INVALID.
		//const COLORREF bk_color_res_1 = ::SetBkColor( m_hDC, clr );
		//ASSERT( bk_color_res_1 != CLR_INVALID );

		//if ( bk_color_res_1 == CLR_INVALID ) {
		//	TRACE( _T( "::SetBkColor( pdc.m_hDC, color ) failed!!\r\n" ) );
		//	}

		////If the string is drawn, the return value [of ExtTextOutW] is nonzero. However, if the ANSI version of ExtTextOut is called with ETO_GLYPH_INDEX, the function returns TRUE even though the function does nothing.
		//VERIFY( ::ExtTextOutW( m_hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL ) );

		fill_solid_RECT( m_hDC, &rc, clr );
		}
	}

/*
From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:688:
#define IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew, class_init) \
	AFX_COMDAT const CRuntimeClass class_name::class##class_name = { \
		#class_name, sizeof(class class_name), wSchema, pfnNew, \
			RUNTIME_CLASS(base_class_name), NULL, class_init }; \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(class_name); }

From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:704:
#define IMPLEMENT_DYNAMIC(class_name, base_class_name) \
	IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, NULL, NULL)

From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:598:
#define RUNTIME_CLASS(class_name) _RUNTIME_CLASS(class_name)

From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:594:
#define _RUNTIME_CLASS(class_name) ((CRuntimeClass*)(&class_name::class##class_name))

Sooo...
	IMPLEMENT_DYNAMIC(CXySlider, CStatic)
		--becomes--
	IMPLEMENT_RUNTIMECLASS(CXySlider, CStatic, 0xFFFF, NULL, NULL)
		--becomes--
IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew, class_init) \
	AFX_COMDAT const CRuntimeClass CXySlider::classCXySlider = { \
		"CXySlider", sizeof(class CXySlider), wSchema, pfnNew, \
			RUNTIME_CLASS(CStatic), NULL, class_init }; \
	CRuntimeClass* CXySlider::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(CXySlider); }
and...
	RUNTIME_CLASS(CStatic)
		--becomes--
	_RUNTIME_CLASS(CStatic)
		--becomes--
	((CRuntimeClass*)(&CStatic::classCStatic))
and...
	RUNTIME_CLASS(CXySlider);
		--becomes--
	_RUNTIME_CLASS(CXySlider)
		--becomes--
	((CRuntimeClass*)(&CXySlider::classCXySlider))

*/
//IMPLEMENT_DYNAMIC(CXySlider, CStatic)
//IMPLEMENT_RUNTIMECLASS(CXySlider, CStatic, 0xFFFF, NULL, NULL)
AFX_COMDAT const CRuntimeClass CXySlider::classCXySlider = {
	"CXySlider" /*m_lpszClassName*/,
	sizeof(class CXySlider) /*m_nObjectSize*/,
	0xFFFF /*wSchema*/,
	nullptr  /*pfnNew*/,
	(const_cast<CRuntimeClass*>(&CStatic::classCStatic)) /*RUNTIME_CLASS(CStatic)*/ /*m_pBaseClass*/,
	nullptr  /*m_pNextClass*/,
	nullptr  /*class_init*/
	};

CRuntimeClass* CXySlider::GetRuntimeClass() const {
	return (const_cast<CRuntimeClass*>(&CXySlider::classCXySlider)); /* RUNTIME_CLASS(CXySlider);*/
	}

void AFXAPI DDX_XySlider( CDataExchange* pDX, INT nIDC, POINT& value ) {
	pDX->PrepareCtrl(nIDC);
	HWND hWndCtrl = nullptr;
	pDX->m_pDlgWnd->GetDlgItem( nIDC, &hWndCtrl );
	if ( pDX->m_bSaveAndValidate ) {
		::SendMessageW( hWndCtrl, XY_GETPOS, 0, reinterpret_cast<LPARAM>(&value) );
		}
	else {
		::SendMessageW( hWndCtrl, XY_SETPOS, 0, reinterpret_cast<LPARAM>(&value) );
		}
	}

void CXySlider::Initialize( ) noexcept {
	if ( ( !m_inited ) && ( ::IsWindow( m_hWnd ) ) ) {
		// Make size odd, so that zero lines are central
		RECT rc = { 0, 0, 0, 0 };

		//"Return value: If [GetWindowRect] succeeds, the return value is nonzero. If [GetWindowRect] fails, the return value is zero. To get extended error information, call GetLastError."
		VERIFY( ::GetWindowRect( m_hWnd, &rc ) );
		hwnd::ScreenToClient(::GetParent(m_hWnd), &rc);
		if ( ( rc.right - rc.left ) % 2 == 0 ) {
			rc.right--;
			}
		if ( ( rc.bottom - rc.top ) % 2 == 0 ) {
			rc.bottom--;
			}
	
		ASSERT( ::IsWindow( m_hWnd ) );

		//If [MoveWindow] succeeds, the return value is nonzero.
		VERIFY( ::MoveWindow( m_hWnd, rc.left, rc.top, ( rc.right - rc.left ), ( rc.bottom - rc.top ), TRUE ) );

		CXySlider::CalcSizes( );

		m_inited = true;
		}
	}

void CXySlider::CalcSizes( ) noexcept {
	ASSERT( ::IsWindow( m_hWnd ) );
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero. To get extended error information, call GetLastError."
	VERIFY( ::GetClientRect( m_hWnd, &m_rcAll ) );

	ASSERT( m_rcAll.left == 0 );
	ASSERT( m_rcAll.top  == 0 );
	ASSERT( ( m_rcAll.right - m_rcAll.left ) % 2 == 1 );
	ASSERT( ( m_rcAll.bottom - m_rcAll.top ) % 2 == 1 );
	ASSERT( ( m_rcAll.right - m_rcAll.left ) >= GRIPPER_RADIUS * 2 );	// Control must be large enough
	ASSERT( ( m_rcAll.bottom - m_rcAll.top ) >= GRIPPER_RADIUS * 2 );

	m_zero.x = ( m_rcAll.right - m_rcAll.left ) / 2;
	m_zero.y = ( m_rcAll.bottom - m_rcAll.top ) / 2;

	m_radius.cx = ( m_rcAll.right - m_rcAll.left ) / 2 - 1;
	m_radius.cy = ( m_rcAll.bottom - m_rcAll.top ) / 2 - 1;

	m_rcInner = m_rcAll;

	//"Return value: If [InflateRect] succeeds, the return value is nonzero. If [InflateRect] fails, the return value is zero."
	VERIFY( ::InflateRect( &m_rcInner, -( GRIPPER_RADIUS - 3 ), -( GRIPPER_RADIUS - 3 ) ) );
	//m_rcInner.DeflateRect( GRIPPER_RADIUS - 3, GRIPPER_RADIUS - 3 );


	m_gripperRadius.cx = GRIPPER_RADIUS;
	m_gripperRadius.cy = GRIPPER_RADIUS;

	m_range.cx = m_radius.cx - m_gripperRadius.cx;
	m_range.cy = m_radius.cy - m_gripperRadius.cy;

	//m_range = m_radius - m_gripperRadius;
	}

void CXySlider::NotifyParent( ) const noexcept {
	NMHDR hdr = { };
	hdr.hwndFrom = m_hWnd;

	/*
	Maybe:
	int GetDlgCtrlID() const throw()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetDlgCtrlID(m_hWnd);
	}
	*/
	const int ID = hwnd::GetDlgCtrlID(m_hWnd);
	hdr.idFrom   = static_cast<UINT_PTR>( ID );
	hdr.code     = XYSLIDER_CHANGED;
	TRACE( _T( "NotifyParent called! Sending WM_NOTIFY!\r\n" ) );
	//CWnd::GetParent( )->SendMessageW( WM_NOTIFY, static_cast<WPARAM>( ID ), reinterpret_cast<LPARAM>(&hdr) );
	::SendMessageW(::GetParent(m_hWnd), WM_NOTIFY, static_cast<WPARAM>(ID), reinterpret_cast<LPARAM>(&hdr));
	}

void CXySlider::PaintBackground( _In_ const HDC hDC, _In_ const HDC hAttribDC) noexcept {
	//ASSERT_VALID( &pdc );
	//ASSERT( pdc.m_hDC != NULL );

	////If [SetBkColor] fails, the return value is CLR_INVALID.
	//const COLORREF color_res_1 = ::SetBkColor( pdc.m_hDC, GetSysColor( COLOR_BTNFACE ) );
	//
	////TODO: check this!
	//ASSERT( color_res_1 != CLR_INVALID );
	//if ( color_res_1 == CLR_INVALID ) {
	//	TRACE( _T( "SetBkColor( pdc.m_hDC, GetSysColor( COLOR_BTNFACE ) ) failed!!\r\n" ) );
	//	}

	////If the string is drawn, the return value [of ExtTextOutW] is nonzero. However, if the ANSI version of ExtTextOut is called with ETO_GLYPH_INDEX, the function returns TRUE even though the function does nothing.
	//VERIFY( ::ExtTextOutW( pdc.m_hDC, 0, 0, ETO_OPAQUE, &m_rcAll, NULL, 0u, NULL ) );

	fill_solid_RECT( hDC, &m_rcAll, ::GetSysColor( COLOR_BTNFACE ) );

	RECT rc = m_rcInner;
	
	ASSERT( hDC != NULL );

	//If [DrawEdge] succeeds, the return value is nonzero. If [DrawEdge] fails, the return value is zero.
	VERIFY( ::DrawEdge( hDC, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST ) );

	//pdc.FillSolidRect( &rc, RGB( 255, 255, 255 ) );

	//ASSERT_VALID( &pdc );
	//ASSERT( pdc.m_hDC != NULL );

	////If [SetBkColor] fails, the return value is CLR_INVALID.
	//const COLORREF color_res_2 = ::SetBkColor( pdc.m_hDC, RGB( 255, 255, 255 ) );
	//
	////TODO: check this!
	//ASSERT( color_res_2 != CLR_INVALID );

	//if ( color_res_2 == CLR_INVALID ) {
	//	TRACE( _T( "::SetBkColor( pdc.m_hDC, RGB( 255, 255, 255 ) ) failed!!\r\n" ) );
	//	}

	////If the string is drawn, the return value [of ExtTextOutW] is nonzero. However, if the ANSI version of ExtTextOut is called with ETO_GLYPH_INDEX, the function returns TRUE even though the function does nothing.
	//VERIFY( ::ExtTextOutW( pdc.m_hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0u, NULL ) );


	fill_solid_RECT( hDC, &rc, RGB( 255, 255, 255 ) );

	//CPen pen( PS_SOLID, 1, ::GetSysColor( COLOR_3DLIGHT ) );
	
	// CreatePen function (wingdi.h): https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createpen
	// If the function succeeds, the return value is a handle that identifies a logical pen.
	// If the function fails, the return value is NULL.
	const HPEN hPen = ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DLIGHT));
	HGDIOBJ_wrapper pen(hPen);

	SelectObject_wrapper sopen( hDC, pen.m_hObject );
	//ASSERT(hAttribDC == nullptr);


	// https://docs.microsoft.com/en-us/cpp/mfc/reference/cdc-class?view=vs-2019#m_hdc
	// "By default, m_hDC is equal to m_hAttribDC, the other device context wrapped by CDC"
	ASSERT(hDC == hAttribDC);
	move_to_coord( hDC, rc.left, m_zero.y, hAttribDC );

	line_to_coord( hDC, rc.right, m_zero.y, hAttribDC );
	
	move_to_coord( hDC, m_zero.x, rc.top, hAttribDC );

	line_to_coord( hDC, m_zero.x, rc.bottom, hAttribDC );

	RECT circle = m_rcAll;

	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
	//circle.DeflateRect( m_gripperRadius );
	VERIFY( ::InflateRect( &circle, -( m_gripperRadius.cx ), -( m_gripperRadius.cy ) ) );

	SelectStockObject_wrapper sobrush( hDC, NULL_BRUSH );

	ASSERT( hDC != NULL );
	//If [Ellipse] succeeds, the return value is nonzero. If [Ellipse] fails, the return value is zero.
	VERIFY( ::Ellipse( hDC, circle.left, circle.top, circle.right, circle.bottom ) );

	//--------------------------------
	if (::GetFocus() == m_hWnd) {
		ASSERT(CWnd::GetFocus() == this);
		//TODO: what function?
		//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
		ASSERT( ::IsWindow( m_hWnd ) );
		VERIFY( ::DrawFocusRect( hDC, &m_rcAll ) );
		}
	}

void CXySlider::PaintGripper( _In_ const HDC m_hDC, _In_ HDC m_hAttribDC) noexcept {
	//ASSERT_VALID( pdc );
	RECT rc = CXySlider::GetGripperRect( );

	COLORREF color_scopeholder = ::GetSysColor( COLOR_BTNFACE );
	if ( m_gripperHighlight ) {
		INT r = GetRValue( color_scopeholder );
		INT g = GetGValue( color_scopeholder );
		INT b = GetBValue( color_scopeholder );;
		r += ( 255 - r ) / 3;
		g += ( 255 - g ) / 3;
		b += ( 255 - b ) / 3;
		color_scopeholder = RGB( r, g, b );
		}

	const COLORREF color = color_scopeholder;

	fill_solid_rectangle( m_hDC, rc, color );

	ASSERT( m_hDC != NULL );

	//If [DrawEdge] succeeds, the return value is nonzero. If [DrawEdge] fails, the return value is zero.
	VERIFY( ::DrawEdge( m_hDC, &rc, EDGE_RAISED, BF_RECT ) );

	//CreatePen function: https://msdn.microsoft.com/en-us/library/dd183509.aspx
	//The CreatePen function creates a logical pen that has the specified style, width, and color.
	//The pen can subsequently be selected into a device context and used to draw lines and curves.
	//If the function succeeds, the return value is a handle that identifies a logical pen.
	//If the function fails, the return value is NULL.
	//When you no longer need the pen, call the DeleteObject function to delete it.
	//TODO: GetSysColorBrush?
	const HPEN hPen = ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));
	/*
	CPen::CPen(int nPenStyle, int nWidth, COLORREF crColor)
	{
		if (!Attach(::CreatePen(nPenStyle, nWidth, crColor)))
			AfxThrowResourceException();
	}
	*/
	if (hPen == NULL) {
		std::terminate();
	}
	HGDIOBJ_wrapper pen(hPen);
	//CPen pen( PS_SOLID, 1, ::GetSysColor( COLOR_3DSHADOW ) );
	SelectObject_wrapper sopen( m_hDC, pen.m_hObject );

	move_to_coord( m_hDC, rc.left, ( rc.top + ( rc.bottom - rc.top ) / 2 ), m_hAttribDC );

	line_to_coord( m_hDC, rc.right, ( rc.top + ( rc.bottom - rc.top ) / 2 ), m_hAttribDC );

	move_to_coord( m_hDC, rc.left + ( rc.right - rc.left ) / 2, rc.top, m_hAttribDC );

	line_to_coord( m_hDC, rc.left + ( rc.right - rc.left ) / 2, rc.bottom, m_hAttribDC );
	}

void CXySlider::DoMoveBy( _In_ const INT cx, _In_ const INT cy ) noexcept {
	m_pos.x += cx;
	CheckMinMax( m_pos.x, -m_range.cx, m_range.cx );

	m_pos.y += cy;
	CheckMinMax( m_pos.y, -m_range.cy, m_range.cy );


	/*
	_AFXWIN_INLINE BOOL CWnd::RedrawWindow(LPCRECT lpRectUpdate, CRgn* prgnUpdate,
		UINT flags)
		{ ASSERT(::IsWindow(m_hWnd)); return ::RedrawWindow(m_hWnd, lpRectUpdate, (HRGN)prgnUpdate->GetSafeHandle(), flags); }

	*/
	//VERIFY( CWnd::RedrawWindow( ) );
	hwnd::RedrawWindow(m_hWnd);
	const POINT oldpos = m_externalPos;
	CXySlider::InternToExtern( );
	if ( ( m_externalPos.x != oldpos.x ) || ( m_externalPos.y != oldpos.y ) ) {
		CXySlider::NotifyParent( );
		}
	}

void CXySlider::Handle_WM_MOUSEMOVE( _In_ const POINT& ptMin, _In_ const POINT& ptMax, _In_ const MSG& msg, _Inout_ POINT& pt0 ) noexcept {
	POINT pt = msg.pt;
	//VERIFY( ::ScreenToClient( m_hWnd, &pt ) );
	hwnd::ScreenToClient(m_hWnd, &pt);
	//ScreenToClient( &pt );

	CheckMinMax( pt.x, ptMin.x, ptMax.x );
	CheckMinMax( pt.y, ptMin.y, ptMax.y );

	const INT dx = pt.x - pt0.x;
	const INT dy = pt.y - pt0.y;

	CXySlider::DoMoveBy( dx, dy );

	pt0 = pt;
	}

void CXySlider::DoDrag( _In_ const POINT point ) noexcept {
	POINT pt0 = point;

	CXySlider::HighlightGripper( true );

	/*
	inline CPoint CRect::CenterPoint() const throw()
	{
		return CPoint((left+right)/2, (top+bottom)/2);
	}	
	*/
	const RECT grip_rect = CXySlider::GetGripperRect( );
	const auto grip_rect_center_x = ( ( grip_rect.left + grip_rect.right ) / 2 );
	const auto grip_rect_center_y = ( ( grip_rect.top + grip_rect.bottom ) / 2 );
	const auto new_point_x = ( pt0.x - grip_rect_center_x );
	const auto new_point_y = ( pt0.y - grip_rect_center_y );
	const CSize inGripper( new_point_x, new_point_y );
	POINT ptMin_holder = {
		( m_zero.x - m_range.cx + inGripper.cx ),
		( m_zero.y - m_range.cy + inGripper.cy )
		};
	const POINT ptMin = ptMin_holder;

	POINT ptMax_holder = {
		( m_zero.x + m_range.cx + inGripper.cx ),
		( m_zero.y + m_range.cy + inGripper.cy )
		};

	POINT ptMax = ptMax_holder;
	/*
	_AFXWIN_INLINE CWnd* CWnd::SetCapture()
		{ ASSERT(::IsWindow(m_hWnd)); return CWnd::FromHandle(::SetCapture(m_hWnd)); }

	*/
	ASSERT(::IsWindow(m_hWnd));

	// SetCapture function (winuser.h): https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setcapture
	// The return value is a handle to the window that had previously captured the mouse. If there is no such window, the return value is NULL.

	//Don't care about return value.
	(void)::SetCapture(m_hWnd);
	//CWnd::SetCapture( );
	do {
		MSG msg;
		if ( !::GetMessageW( &msg, nullptr, 0, 0 ) ) {
			break;
			}

		if ( msg.message == WM_LBUTTONUP ) {
			break;
			}
		if ( ::GetCapture( ) != m_hWnd ) {
			ASSERT(CWnd::GetCapture() != this);
			break;
			}

		if ( msg.message == WM_MOUSEMOVE ) {
			CXySlider::Handle_WM_MOUSEMOVE( ptMin, ptMax, msg, pt0 );
			}
		else {
			// Return value for DispatchMessageW is generally ignored.
			(void)::DispatchMessageW( &msg );
			}
#pragma warning(suppress:4127)//conditional expression is constant
		} while ( true );

	VERIFY( ::ReleaseCapture( ) );

	CXySlider::HighlightGripper( false );
	}

void CXySlider::DoPage( _In_ const POINT point ) noexcept {
	POINT _m_zero_m_pos_scope_holder = {
		( m_zero.x + m_pos.x ),
		( m_zero.y + m_pos.y )
		};


	const POINT& _m_zero_m_pos_ = _m_zero_m_pos_scope_holder;

	POINT _point_minus_m_zero_m_pos = { point.x, point.y };

	_point_minus_m_zero_m_pos.x -= _m_zero_m_pos_.x;
	_point_minus_m_zero_m_pos.y -= _m_zero_m_pos_.y;

	SIZE sz_holder = { _point_minus_m_zero_m_pos.x, _point_minus_m_zero_m_pos.y };

	const SIZE& sz = sz_holder;

	ASSERT( sz.cx != 0 || sz.cy != 0 );

	const auto len = ::sqrt( static_cast<DOUBLE>( sz.cx ) * static_cast<DOUBLE>( sz.cx ) + static_cast<DOUBLE>( sz.cy ) * static_cast<DOUBLE>( sz.cy ) );

	const auto dx = static_cast<INT>( 10 * static_cast<std::int64_t>(sz.cx) / len );
	const auto dy = static_cast<INT>( 10 * static_cast<std::int64_t>(sz.cy) / len );

	CXySlider::DoMoveBy( dx, dy );
	}

void CXySlider::HighlightGripper( _In_ const bool on ) noexcept {
	m_gripperHighlight = on;
	hwnd::RedrawWindow(m_hWnd);
	//VERIFY( CWnd::RedrawWindow( ) );
	}

void CXySlider::RemoveTimer( ) noexcept {
	if ( m_timer != 0 ) {
		ASSERT( ::IsWindow( m_hWnd ) );
		
		//If [KillTimer] succeeds, the return value is nonzero. If [KillTimer] fails, the return value is zero. To get extended error information, call GetLastError.
		VERIFY( ::KillTimer( m_hWnd, m_timer ) );
		}
	m_timer = 0;
	}


BEGIN_MESSAGE_MAP(CXySlider, CStatic)
	ON_WM_DESTROY()
	ON_WM_GETDLGCODE()
	ON_WM_NCHITTEST()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_MESSAGE(XY_SETPOS, &( CXySlider::OnSetPos ) )
	ON_MESSAGE(XY_GETPOS, &( CXySlider::OnGetPos ) )
END_MESSAGE_MAP()

void CXySlider::OnPaint( ) {
	CXySlider::Initialize( );
	const INT w = ( m_rcAll.right - m_rcAll.left );
	const INT h = ( m_rcAll.bottom - m_rcAll.top );


	PAINTSTRUCT ps = { 0 };
	const HDC dc = hwnd::BeginPaint(m_hWnd, &ps);
	auto paint_guard = WDS_SCOPEGUARD_INSTANCE([&]() { hwnd::EndPaint(m_hWnd, ps); });
	const HDC dcmem = gdi::CreateCompatibleDeviceContext(dc);
	auto paint_mem_guard = WDS_SCOPEGUARD_INSTANCE([&]() { gdi::DeleteDeviceContext(dcmem); });
	const HGDIOBJ bm = gdi::CreateCompatibleBitmap(dc, w, h);
	HGDIOBJ_wrapper bm_(bm);

	/*
	_AFXWIN_INLINE BOOL CBitmap::CreateCompatibleBitmap(CDC* pDC, int nWidth, int nHeight)
		{ return Attach(::CreateCompatibleBitmap(pDC->m_hDC, nWidth, nHeight)); }
	*/
	SelectObject_wrapper sobm( dcmem, bm );

	// https://docs.microsoft.com/en-us/cpp/mfc/reference/cdc-class?view=vs-2019#m_hdc
	// "By default, m_hDC is equal to m_hAttribDC, the other device context wrapped by CDC"
	CXySlider::PaintBackground( dcmem, dcmem );

	// PaintValues(&dcmem); This is too noisy
	CXySlider::PaintGripper( dcmem, dcmem );

	//VERIFY( dc.BitBlt( 0, 0, w, h, &dcmem, 0, 0, SRCCOPY ) );

	VERIFY(::BitBlt(dc, 0, 0, w, h, dcmem, 0, 0, SRCCOPY));
	}


void CXySlider::OnKeyDown( UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/ ) {
	switch ( nChar )
	{
		case VK_LEFT:
			return CXySlider::DoMoveBy( -1,  0 );
		case VK_RIGHT:
			return CXySlider::DoMoveBy(  1,  0 );
		case VK_UP:
			return CXySlider::DoMoveBy(  0, -1 );
		case VK_DOWN:
			return CXySlider::DoMoveBy(  0,  1 );
	}
	}

void CXySlider::OnLButtonDown( UINT /*nFlags*/, CPoint point ) {
	//CWnd::SetFocus( );
	::SetFocus(m_hWnd);

	const RECT rc = GetGripperRect( );

	if ( ::PtInRect( &rc, point ) ) {
		return CXySlider::DoDrag( point );
		}
	CXySlider::DoPage( point );
	CXySlider::InstallTimer( );
	}

void CXySlider::OnLButtonDblClk( UINT /*nFlags*/, CPoint point ) {
	//CWnd::SetFocus( );
	::SetFocus(m_hWnd);

	const RECT grip_rect = CXySlider::GetGripperRect( );
	if ( ::PtInRect( &grip_rect, point ) ) {
		return CXySlider::DoMoveBy( -m_pos.x, -m_pos.y );
		}
	CXySlider::DoPage( point );
	CXySlider::InstallTimer( );
	}

void CXySlider::OnTimer( UINT_PTR /*nIDEvent*/ ) {
	POINT point;
	VERIFY( ::GetCursorPos( &point ) );
	ASSERT( ::IsWindow( m_hWnd ) );
	
	//VERIFY( ::ScreenToClient( m_hWnd, &point ) );
	hwnd::ScreenToClient(m_hWnd, &point);

	const RECT rc = CXySlider::GetGripperRect( );
	if ( !::PtInRect( &rc, point ) ) {
		CXySlider::DoPage( point );
		}
	}

void CXySlider::SetPos( const POINT pt ) {
	CXySlider::Initialize( );
	m_externalPos = pt;
	CXySlider::ExternToIntern( );

	hwnd::InvalidateErase(m_hWnd);
	}


#else

#endif