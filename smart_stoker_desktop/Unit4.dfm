object Form4: TForm4
  Left = 215
  Top = 131
  BorderStyle = bsDialog
  Caption = #1043#1088#1072#1085#1080#1094#1099' '#1075#1088#1072#1092#1080#1082#1072
  ClientHeight = 144
  ClientWidth = 194
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 10
    Top = 40
    Width = 59
    Height = 20
    Caption = #1053#1080#1078#1085#1103#1103
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlue
    Font.Height = -16
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label2: TLabel
    Left = 10
    Top = 10
    Width = 63
    Height = 20
    Caption = #1042#1077#1088#1093#1085#1103#1103
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clRed
    Font.Height = -16
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Button1: TButton
    Left = 55
    Top = 105
    Width = 81
    Height = 25
    Caption = #1059#1089#1090#1072#1085#1086#1074#1080#1090#1100
    TabOrder = 0
    OnClick = Button1Click
  end
  object Edit1: TEdit
    Left = 80
    Top = 10
    Width = 46
    Height = 21
    MaxLength = 3
    TabOrder = 1
    Text = '0'
  end
  object Edit2: TEdit
    Left = 80
    Top = 40
    Width = 46
    Height = 21
    MaxLength = 3
    TabOrder = 2
    Text = '0'
  end
  object CheckBox1: TCheckBox
    Left = 10
    Top = 75
    Width = 176
    Height = 17
    Caption = #1040#1074#1090#1086#1084#1072#1089#1096#1090#1072#1073#1080#1088#1086#1074#1072#1085#1080#1077
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 3
    OnClick = CheckBox1Click
  end
  object UpDown1: TUpDown
    Left = 126
    Top = 10
    Width = 16
    Height = 21
    Associate = Edit1
    Min = -90
    Max = 125
    Position = 0
    TabOrder = 4
    Wrap = False
  end
  object UpDown2: TUpDown
    Left = 126
    Top = 40
    Width = 16
    Height = 21
    Associate = Edit2
    Min = -90
    Max = 125
    Position = 0
    TabOrder = 5
    Wrap = False
  end
end
