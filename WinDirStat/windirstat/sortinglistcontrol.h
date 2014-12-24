// sortinglistcontrol.h	- Declaration of CSortingListItem and CSortingListControl
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2004 Bernhard Seifert
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.				
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: bseifert@users.sourceforge.net, bseifert@daccord.net
//
// Last modified: $Date$

#ifndef SORTINGLISTCONTROL_H
#define SORTINGLISTCONTROL_H
#else
#error ass
#endif


#pragma once

#include "stdafx.h"
//#include "globalhelpers.h"

// SSorting. A sorting specification. We sort by column1, and if two items equal in column1, we sort them by column2.
struct SSorting {
	SSorting( ) : column1( column::COL_NAME ), column2( column::COL_NAME ), ascending1( false ), ascending2( true ) { }
	_Field_range_( 0, 8 ) column::ENUM_COL  column1;
	_Field_range_( 0, 8 ) column::ENUM_COL  column2;
	                      bool              ascending2 : 1;
	                      bool              ascending1 : 1;
	};


class COwnerDrawnListItem;

// CSortingListItem. An item in a CSortingListControl.
//class CSortingListItem {
//public:
//	//virtual CString GetText ( _In_ _In_range_( 0, INT32_MAX ) const INT subitem ) const = 0;
//
//	//virtual INT Compare( _In_ const CSortingListItem* const other, _In_ const INT subitem ) const = 0;
//	
//	
//#ifdef DRAW_ICONS
//	virtual INT GetImage    (                        ) const = 0;
//#endif
//	};

// CSortingListControl. The base class for all our ListControls.
// The lParams of the items are pointers to CSortingListItems.
// The items use LPSTR_TEXTCALLBACK and I_IMAGECALLBACK.
// And the items can compare to one another.
// CSortingListControl maintains a SSorting and handles clicks on the header items. It also indicates the sorting to the user by adding a "<" or ">" to the header items.
class CSortingListControl : public CListCtrl {
	DECLARE_DYNAMIC(CSortingListControl)
public:
	CSortingListControl( _In_z_ PCWSTR name ) : m_name( name ), m_indicatedColumn( -1 ) { }

	void LoadPersistentAttributes        (                                                                                                                      );
	void SavePersistentAttributes        (                                                                                                                      );
	void SortItems                       (                                                                                                                      );
  //void GetSorting                      (            INT&      sortColumn1,            bool& ascending1,           INT& sortColumn2,           INT& ascending2 );
	void AddExtendedStyle                ( _In_ const DWORD     exStyle );
  //void RemoveExtendedStyle             ( _In_ const DWORD     exStyle );
	void InsertListItem                  ( _In_ const INT_PTR   i, _In_ const      COwnerDrawnListItem* const item );
_Must_inspect_result_ 
	COwnerDrawnListItem* GetSortingListItem( _In_ const INT i );

	void SetSorting( _In_ const column::ENUM_COL       sortColumn, _In_ const bool ascending ) {
		m_sorting.ascending2 = m_sorting.ascending1;
		m_sorting.column1    = sortColumn;
		m_sorting.column2    = m_sorting.column1;
		m_sorting.ascending1 = ascending;
		}

	bool AscendingDefault( _In_ const column::ENUM_COL column ) const {
		return GetAscendingDefault( column );
		}

private:
	// Overridables
	virtual bool GetAscendingDefault ( _In_ const column::ENUM_COL column ) const {
		UNREFERENCED_PARAMETER( column );
		return true;
		}

	


	            _Field_z_ PCWSTR      m_name;	 // for persistence
public:
						  SSorting    m_sorting;
private:
	_Field_range_( 0, 8 ) std::int8_t m_indicatedColumn;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLvnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHdnItemclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHdnItemdblclick( NMHDR* pNMHDR, LRESULT* pResult ) {
		OnHdnItemclick( pNMHDR, pResult );
		}
	afx_msg void OnDestroy( );
	};


// $Log$
// Revision 1.4  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
