#include "pch.h"
#include <SDK/preferences_page.h>
#include <SDK/cfg_var.h>
#include <helpers/helpers.h>

#if defined(_WIN32) || defined(__MINGW32__)
#include <helpers/atl-misc.h>
#include "resource.h"
#include <helpers/DarkMode.h>
#include "foo_rnd_pause.h"
#endif

// Define GUIDs for the plugin and settings
static const GUID guid_cfg_myplugin = { 0xf1968066, 0x940d, 0x4d50, { 0x8a, 0xb7, 0x16, 0x88, 0xc5, 0x5e, 0x6a, 0xbc } };
static const GUID guid_cfg_myplugin_setting = { 0xea4b5b90, 0x81a6, 0x4aa0, { 0x98, 0x81, 0x14, 0x5b, 0x1b, 0x19, 0x8c, 0x54 } };
static const GUID guid_rnd_min_sec = { 0xea4b5b90, 0x81a6, 0x4aa0, { 0x98, 0x81, 0x14, 0x5b, 0x1b, 0x19, 0x8c, 0x54 } };
static const GUID guid_rnd_max_sec = { 0x15fdcd56, 0x917d, 0x4491, { 0xb8, 0xd1, 0x41, 0x29, 0x75, 0xe1, 0x13, 0xe7 } };
static const GUID guid_check_fix_pause = { 0xe199c40e, 0x1891, 0x471f, { 0xa8, 0x6f, 0x18, 0xcc, 0xdc, 0x11, 0xc7, 0x91 } };
static const GUID guid_apply_to_nth_tracks = { 0x9d7e7979, 0x2069, 0x442b, { 0xa2, 0x28, 0xd0, 0x20, 0xed, 0xd5, 0x9a, 0x68 } };

// default values
constexpr int rnd_min_sec = 1;
constexpr int rnd_max_sec = 2;
constexpr int apply_to_nth_tracks = 1;
constexpr bool check_fix_pause = false;

namespace foo_rnd_pause {
	cfg_int cfg_rnd_min_sec(guid_rnd_min_sec, rnd_min_sec);
	cfg_int cfg_rnd_max_sec(guid_rnd_max_sec, rnd_max_sec);
	cfg_int cfg_apply_to_nth_tracks(guid_apply_to_nth_tracks, apply_to_nth_tracks);
	cfg_bool cfg_check_fix_pause(guid_check_fix_pause, check_fix_pause);
}

using namespace foo_rnd_pause;

#if defined(_WIN32) || defined(__MINGW32__)
class CMyPreferences : public CDialogImpl<CMyPreferences>, public preferences_page_instance {
public:
	//Constructor - invoked by preferences_page_impl helpers - don't do Create() in here, preferences_page_impl does this for us
	CMyPreferences(preferences_page_callback::ptr callback) : m_callback(callback) {}

	//Note that we don't bother doing anything regarding destruction of our class.
	//The host ensures that our dialog is destroyed first, then the last reference to our preferences_page_instance object is released, causing our object to be deleted.

	enum { IDD = IDD_MYPREFERENCES };//dialog resource ID
	// preferences_page_instance methods (not all of them - get_wnd() is supplied by preferences_page_impl helpers)
	t_uint32 get_state();
	void apply();
	void reset();


	//WTL message map
	BEGIN_MSG_MAP_EX(CMyPreferences)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_HANDLER_EX(IDC_SETTING1, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER_EX(IDC_SETTING2, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER_EX(IDC_SETTING3, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER_EX(IDC_CHECK1, BN_CLICKED, OnEditChange)
		END_MSG_MAP()
private:
	BOOL OnInitDialog(CWindow, LPARAM);
	void OnEditChange(UINT, int, CWindow);
	bool HasChanged();
	void OnChanged();

	const preferences_page_callback::ptr m_callback;
	fb2k::CDarkModeHooks m_dark;// Dark mode hooks object, must be a member of dialog class.
};

BOOL CMyPreferences::OnInitDialog(CWindow, LPARAM) {
	m_dark.AddDialogWithControls(*this);// Enable dark mode. One call does it all, applies all relevant hacks automatically

	SetDlgItemInt(IDC_SETTING1, (INT)foo_rnd_pause::cfg_rnd_min_sec, FALSE);
	SetDlgItemInt(IDC_SETTING2, (INT)foo_rnd_pause::cfg_rnd_max_sec, FALSE);
	SetDlgItemInt(IDC_SETTING3, (INT)foo_rnd_pause::cfg_apply_to_nth_tracks, FALSE);
	CheckDlgButton(IDC_CHECK1, (UINT)foo_rnd_pause::cfg_check_fix_pause);

	return FALSE;
}

void CMyPreferences::OnEditChange(UINT, int, CWindow) {
	// not much to do here
	OnChanged();
}

t_uint32 CMyPreferences::get_state() {
	// IMPORTANT: Always return dark_mode_supported - tell foobar2000 that this preferences page is dark mode compliant.
	t_uint32 state = preferences_state::resettable | preferences_state::dark_mode_supported;
	if (HasChanged()) state |= preferences_state::changed;

	return state;
}

void CMyPreferences::reset() {
	// Reset the actual config variables to their default values
	foo_rnd_pause::cfg_rnd_min_sec = 1;
	foo_rnd_pause::cfg_rnd_max_sec = 2;
	foo_rnd_pause::cfg_apply_to_nth_tracks = 1;
	foo_rnd_pause::cfg_check_fix_pause = false;

	SetDlgItemInt(IDC_SETTING1, foo_rnd_pause::cfg_rnd_min_sec, FALSE);
	SetDlgItemInt(IDC_SETTING2, foo_rnd_pause::cfg_rnd_max_sec, FALSE);
	SetDlgItemInt(IDC_SETTING3, foo_rnd_pause::cfg_apply_to_nth_tracks, FALSE);
	CheckDlgButton(IDC_CHECK1, (UINT)foo_rnd_pause::cfg_check_fix_pause);

	OnChanged();
}

void CMyPreferences::apply() {
	foo_rnd_pause::cfg_rnd_min_sec = GetDlgItemInt(IDC_SETTING1, NULL, FALSE);
	foo_rnd_pause::cfg_rnd_max_sec = GetDlgItemInt(IDC_SETTING2, NULL, FALSE);
	foo_rnd_pause::cfg_apply_to_nth_tracks = GetDlgItemInt(IDC_SETTING3, NULL, FALSE);
	foo_rnd_pause::cfg_check_fix_pause = (bool)IsDlgButtonChecked(IDC_CHECK1);

	OnChanged(); //our dialog content has not changed but the flags have - our currently shown values now match the settings so the apply button can be disabled
}

bool CMyPreferences::HasChanged() {
	//returns whether our dialog content is different from the current configuration (whether the apply button should be enabled or not)

	return
		GetDlgItemInt(IDC_SETTING1, NULL, FALSE) != foo_rnd_pause::cfg_rnd_min_sec
		|| GetDlgItemInt(IDC_SETTING2, NULL, FALSE) != foo_rnd_pause::cfg_rnd_max_sec
		|| GetDlgItemInt(IDC_SETTING3, NULL, FALSE) != foo_rnd_pause::cfg_apply_to_nth_tracks
		|| (bool)IsDlgButtonChecked(IDC_CHECK1) != foo_rnd_pause::cfg_check_fix_pause
		;
}

void CMyPreferences::OnChanged() {
	//tell the host that our state has changed to enable/disable the apply button appropriately.
	m_callback->on_state_changed();
}

class preferences_page_myimpl : public preferences_page_impl<CMyPreferences> {
	// preferences_page_impl<> helper deals with instantiation of our dialog; inherits from preferences_page_v3.
public:
	const char* get_name() { return "foo_rnd_pause Settings"; }
	GUID get_guid() {
		return GUID{ 0xe344d696, 0x9b3f, 0x43b0, { 0xbb, 0x69, 0xa4, 0x3, 0x82, 0x52, 0xb, 0x2e } };
	}
	GUID get_parent_guid() { return guid_tools; }
};

static preferences_page_factory_t<preferences_page_myimpl> g_preferences_page_myimpl_factory;
#endif // _WIN32

DECLARE_COMPONENT_VERSION(
	"foo_rnd_pause",
	"0.1",
	"foobar2000 component."
);

VALIDATE_COMPONENT_FILENAME("foo_rnd_pause.dll");

// Debugging-Informationen
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
