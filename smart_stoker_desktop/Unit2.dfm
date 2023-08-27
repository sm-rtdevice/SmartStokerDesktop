object Form2: TForm2
  Left = 741
  Top = 255
  BorderStyle = bsDialog
  Caption = 'DS1820 SN:'
  ClientHeight = 125
  ClientWidth = 221
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  PixelsPerInch = 96
  TextHeight = 13
  object ListBox1: TListBox
    Left = 5
    Top = 5
    Width = 211
    Height = 116
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ItemHeight = 20
    ParentFont = False
    TabOrder = 0
    OnDblClick = ListBox1DblClick
  end
end
