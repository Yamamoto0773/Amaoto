﻿#pragma once


#include "FontTexture.hpp"
#include "DirectXFontBase.hpp"
#include <array>


namespace dx9 {

	// 文字列描画クラス
	class DirectXFont : public font::DirectXFontBase {

		static const int CHARACTER_MAXCNT = 1024;	// 文字列の最大文字数

	private:
		HFONT					hFont;			// 設定中のフォントのハンドル
		AntialiasLevel			antialiasLv;

		TEXTMETRICW				tm;
	
		std::vector<std::unique_ptr<texture::FontTextureW>> texRes;

		std::array<wchar_t, CHARACTER_MAXCNT+1> workBuf;
		

		
		

	public:
		DirectXFont();
		~DirectXFont();


		bool Create(
			const std::wstring&	fontName,
			size_t			fontSize,
			FontWeight		fontWeight = FontWeight::NORMAL,
			bool			isItalic = false,
			bool			isUnderLine = false,
			bool			isStrikeOut = false,
			AntialiasLevel	level = AntialiasLevel::_15STEPS
			);


		// 文字列テクスチャを明示的に作成
		bool StoreFontTex(const wchar_t* wstr);
	
		


		//////////////////////////////////////////////
		// 文字描画

		// シンプル描画
		bool Draw(
			float x,
			float y,			
			const wchar_t* s, ...
			);

		// 指定領域内へ文字描画
		bool DrawInRect(
			RectF &rect,	
			const wchar_t* s, ...
			);

		// 文字送り
		// note:最後の文字まで描画する時は，drawChCntに負数を指定
		bool Draw(
			float x,
			float y,			
			size_t startCharCnt,
			int drawCharCnt,
			const wchar_t* s, ...
			);

		// 文字送りx指定領域内描画
		// note:最後の文字まで描画する時は，drawChCntに負数を指定
		bool DrawInRect(
			RectF &rect,		
			size_t startCharCnt,
			int drawCharCnt,
			const wchar_t* s, ...
			);


		// カスタム描画
		bool DrawInRect(
			RectF &rect,
			size_t startCharCnt,
			int drawCharCnt,
			size_t fontSize,
			const wchar_t* s, ...
			);


		// フォントテクスチャを開放する (デストラクタで自動的に呼び出されます。明示的に呼び出す必要はありません。
		bool Delete(unsigned int code);
		bool DeleteAll();


	

		

	private:

		// 描画を管理する関数
		bool DrawFont(
			RectF &rect,
			bool isAlign,
			size_t startCharCnt,
			int drawCharCnt,
			size_t fontSize,
			const wchar_t* s,
			va_list vlist
			);


	

		// 文字列strのoffset番目の文字から，長さlimitに1行で入る文字数を取得
		// 1行の長さは，lengthに書き込まれる
		// limit < 0.0fの場合，長さの指定を無視する
		int GetStrLength(const wchar_t* str, size_t offset, int letterSpace, size_t fontSize, float limit, int &length);


	};






}