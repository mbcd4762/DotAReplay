#ifndef __UI_REPLAY_PRESENT__
#define __UI_REPLAY_PRESENT__

#include "ui/replaywnd.h"
#include "script/editor.h"
#include "frameui/controlframes.h"

class ReplayPresentTab : public ReplayTab
{
  static INT_PTR CALLBACK ScriptGenDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
  ComboFrame* presets;
  ButtonFrame* savePreset;
  ButtonFrame* delPreset;

  ScriptEditor* editor;
  EditFrame* result;

  void updateValidity();

  uint32 onMessage(uint32 message, uint32 wParam, uint32 lParam);
  void onSetReplay();
public:
  ReplayPresentTab(Frame* parent);
};

#endif // __UI_REPLAY_PRESENT__
