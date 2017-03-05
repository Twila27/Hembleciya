#include "Engine/Core/TheConsole.hpp"
#include <stdarg.h>
#include "Engine/Input/TheInput.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Core/Command.hpp"


//--------------------------------------------------------------------------------------------------------------
TheConsole* g_theConsole = nullptr;
STATIC Rgba TheConsole::DEFAULT_COLOR;


//--------------------------------------------------------------------------------------------------------------
void ShowHelp( Command& /*args*/ )
{
	//  Example use of args:
	// 	int arg0;
	// 	const char* arg1;

	// 	if ( args.GetNextInt( &arg0, 0 ) && args.GetNextString( &arg1, nullptr ) )
	// 	{
	// 		//Success! Do something with these two args.
	// 	}
	// 	else
	// 	{
	// 		DebuggerPrintf( "help follows format: help <int> <string>" );
	// 	}
	// 
	// 	Rgba color;
	// 	args.GetNextColor( &color, Rgba::WHITE ); //Second arg for default if &color returns with zilch.

	g_theConsole->Printf( "Available, case-insensitive commands: " );
	auto cmdIterEnd = s_theConsoleCommands.end();
	for ( auto cmdIter = s_theConsoleCommands.begin(); cmdIter != cmdIterEnd; ++cmdIter )
	{
		std::pair< std::string, ConsoleCommandCallback* > cmd = *cmdIter;
		g_theConsole->Printf( "%s", cmd.first.c_str() );
	}

	//Printing all because preferred to keep usage text inside command implementation.
}


//--------------------------------------------------------------------------------------------------------------
void ClearLog( Command& /*args*/ )
{
	g_theConsole->ClearConsoleLog();
}


//--------------------------------------------------------------------------------------------------------------
void CloseConsole( Command& /*args*/ )
{
	g_theConsole->HideConsole();
}


//--------------------------------------------------------------------------------------------------------------
void SetColor( Command& args )
{
	Rgba out;

	bool success = args.GetNextColor( &out, Rgba::WHITE );
	if ( !success )
	{
		g_theConsole->Printf( "Incorrect arguments, assigning default value." );
		g_theConsole->Printf( "Usage: SetColor <0-255> <0-255> <0-255> <0-255>" );
	}

	g_theConsole->SetTextColor( out );
}


//--------------------------------------------------------------------------------------------------------------
void FindString( Command& args )
{
	std::string phraseToSearch = args.GetArgsString();
	bool success = ( phraseToSearch != "" );
	if ( !success )
	{
		g_theConsole->ShouldPulseSearchResults( false, "" );
		g_theConsole->Printf( "Stopped search result highlighting." );
		g_theConsole->Printf( "Usage: find <unquoted phrase, or nothing to dehighlight>" );
	}
	else
	{
		g_theConsole->ShouldPulseSearchResults( true, phraseToSearch );
	}
}


//-----------------------------------------------------------------------------------------------
void TheConsole::Printf( const char* messageFormat, ... )
{
	static unsigned int numInvocation = 0;
	//Duplicating code from DebuggerPrintf() because it appears I'd need to do the same thing just to pass it to Stringf().
	
	const int MESSAGE_MAX_LENGTH = 2048;
	char messageLiteral[ MESSAGE_MAX_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, messageFormat );
	vsnprintf_s( messageLiteral, MESSAGE_MAX_LENGTH, _TRUNCATE, messageFormat, variableArgumentList );
	va_end( variableArgumentList );
	messageLiteral[ MESSAGE_MAX_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	ROADMAP( "Make it push back a bool--probably turn into m_storedText into a container of structs--that flag whether up/down skips it." );
	std::string finalMessage = std::to_string( ++numInvocation );
	finalMessage += ": ";
	finalMessage += messageLiteral;
	m_storedText.push_back( std::pair< std::string, Rgba>( finalMessage, m_currentColor ) );

	m_newestStoredTextIndexToRender = 0; // (int)m_storedText.size();
}



//--------------------------------------------------------------------------------------------------------------
void TheConsole::Render() //Recall up is +y.
{
	if ( !m_isVisible )
		return;

	ROADMAP( "Cleanup Macro via Enum on TheRenderer" );
	//0x0302 - GL_SRC_ALPHA
	//0x0303 - GL_ONE_MINUS_SRC_ALPHA
	g_theRenderer->SetBlendFunc( 0x0302, 0x0303 );

	g_theRenderer->EnableBackfaceCulling( false ); //Console needs it.
	g_theRenderer->EnableDepthTesting( false ); //Drawing this on top of everything else.

	Rgba bgColor = Rgba::BLACK;
	bgColor.alphaOpacity = 172;
	Rgba fgColor = Rgba::DARK_GRAY;
	fgColor.alphaOpacity = 172;

	//Prompt box is what you type into, log box is what contains all the logged sentences.
	ROADMAP( " A way to render prompt box above the log box." );

	//Sizes for the command prompt are fixed to the active font.
	float consoleX = static_cast<float> ( m_consoleX );
	float consoleY = static_cast<float> ( m_consoleY );
	float consoleWidth = static_cast<float> ( m_consoleWidth );
	float heightOfOneLinePx = m_currentFont->GetTallestGlyphHeightPx() * m_currentScale; //QUESTION: couldn't we just cache this in the obj until a font change occurs to not have to do all this and the below recalc?
	float spacingPx = heightOfOneLinePx * .1f;
	float promptBoxHeight = ( m_showPromptBox ? heightOfOneLinePx : 0.f );
	m_currentPromptBoxTopLeft.x = consoleX + spacingPx;
	m_currentPromptBoxTopLeft.y = consoleY + spacingPx + promptBoxHeight;
	Vector2f currentPromptBoxBottomRight = Vector2f( consoleWidth - spacingPx, consoleY + promptBoxHeight );

	//Sizes for the log: for each stored line of text add heightOfOneLinePx under the max, at which point cut the log off.
	Vector2f currentLogBoxBottomRight = Vector2f( currentPromptBoxBottomRight.x, m_currentPromptBoxTopLeft.y + ( spacingPx * 2.f ) );
	float totalHeightOfLinesInLogPx = 0.f;
	for ( m_storedLinesToShowCount = 0; m_storedLinesToShowCount < m_storedText.size(); m_storedLinesToShowCount++ )
	{
		if ( totalHeightOfLinesInLogPx + heightOfOneLinePx < m_maxConsoleHeightAsScreenPercentage )
			totalHeightOfLinesInLogPx += heightOfOneLinePx;
		else break;
	}
	m_currentLogBoxTopLeft = Vector2f( m_currentPromptBoxTopLeft.x, currentLogBoxBottomRight.y + totalHeightOfLinesInLogPx + spacingPx + consoleY );

	//Background first: just one amount of spacing above the top of the text entry log's box.
	g_theRenderer->DrawAABB( TheRenderer::AS_QUADS, AABB2f( Vector2f( m_currentLogBoxTopLeft.x, m_currentLogBoxTopLeft.y + spacingPx ), Vector2f( (float)m_consoleWidth, consoleY ) ), bgColor );

	//Then the text fields.
	g_theRenderer->DrawShadedAABB( TheRenderer::AS_QUADS, AABB2f( m_currentLogBoxTopLeft, currentLogBoxBottomRight ), bgColor, bgColor, fgColor, fgColor );
	if ( m_showPromptBox )
	{
		g_theRenderer->DrawAABB( TheRenderer::AS_QUADS, AABB2f( m_currentPromptBoxTopLeft, currentPromptBoxBottomRight ), fgColor );

		//Actual input text so far.
		g_theRenderer->DrawTextProportional2D( Vector2f( m_currentPromptBoxTopLeft.x + ( spacingPx * 2.f ), m_currentPromptBoxTopLeft.y - spacingPx ), m_currentPromptString, m_currentScale, m_currentFont, m_currentColor );

		//"Blinking" cursor.
		Rgba caretColor = Rgba( m_currentColor.red, m_currentColor.green, m_currentColor.blue, static_cast<byte_t>( RangeMap( m_caretAlphaCounter, 0.f, 1.f, 0.f, 255.f ) ) );
		float caretX = m_currentPromptBoxTopLeft.x + g_theRenderer->CalcTextPxWidthUpToIndex( m_currentPromptString, m_caretPosInInputString ); //Draw blinking underscore here.

		if ( m_caretPosInInputString == (int)m_currentPromptString.size() )
			g_theRenderer->DrawTextProportional2D( Vector2f( caretX, heightOfOneLinePx + consoleY ), "_", m_currentScale, m_currentFont, caretColor );
		else
			g_theRenderer->DrawTextProportional2D( Vector2f( caretX, heightOfOneLinePx + consoleY ), "|", m_currentScale, m_currentFont, caretColor );
	}

	//Print text from earlier stored entries: walk from the oldest one shown to the end, iterating forward.
	if ( m_storedLinesToShowCount <= 0 ) return; //No stored text lines to print.
	auto storedTextIterEnd = m_storedText.cend(); //This is the newest, bottom-most stored text line shown.
	auto storedTextIterStart = m_storedText.cend() - m_storedLinesToShowCount; //This is the oldest.
	if ( m_storedLinesToShowCount + m_newestStoredTextIndexToRender <= m_storedText.size() )
	{
		storedTextIterEnd -= m_newestStoredTextIndexToRender;
		storedTextIterStart -= m_newestStoredTextIndexToRender;
	}
	Vector2f positionInLog = Vector2f( consoleX + spacingPx * 2.f, m_currentLogBoxTopLeft.y - spacingPx - consoleY );
	for ( auto storedTextIter = storedTextIterStart; storedTextIter != storedTextIterEnd; ++storedTextIter )
	{
		std::pair< std::string, Rgba > storedLine = *storedTextIter;
		g_theRenderer->DrawTextProportional2D( positionInLog, storedLine.first, m_currentScale, m_currentFont, storedLine.second );
		positionInLog.y -= heightOfOneLinePx; //Move caret down to next line.
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheConsole::RegisterCommand( const std::string& name, ConsoleCommandCallback* cb ) //See RegisterCommandHelper().
{
	if ( s_theConsoleCommands.count( GetAsLowercase( name ) ) != 0 ) //Already exists.
		return;
	else
		s_theConsoleCommands[ GetAsLowercase( name ) ] = cb;
}


//--------------------------------------------------------------------------------------------------------------
void TheConsole::RunCommand( const std::string& fullCommandString )
{
	Command com = Command( fullCommandString ); //Note that we don't yet know if it's registered, hence below find().

												//Allows case insensitivity.
	auto outCommandIter = s_theConsoleCommands.find( GetAsLowercase( com.GetCommandName() ) );
	if ( outCommandIter != s_theConsoleCommands.end() )
	{
		ConsoleCommandCallback* functorToRun = ( *outCommandIter ).second;
		functorToRun( com ); //Runs the search result, because it's a functor.
	}
	else g_theConsole->Printf( "Command not found." );
}


//--------------------------------------------------------------------------------------------------------------
void TheConsole::Update( float deltaSeconds )
{
	if ( g_theInput->WasKeyPressedOnce( KEY_TO_OPEN_CONSOLE ) )
		m_isVisible = !m_isVisible;

	if ( m_isVisible )
	{
		m_caretAlphaCounter += deltaSeconds;
		m_caretAlphaCounter = WrapNumberWithinCircularRange( m_caretAlphaCounter, 0.f, 1.f );


		int mouseWheelDelta = g_theInput->GetMouseWheelDelta();
		if ( mouseWheelDelta != 0 ) //No increments.
		{
			if ( mouseWheelDelta < 0 && m_newestStoredTextIndexToRender - 1 >= 0 )
				m_newestStoredTextIndexToRender--;
			else if ( mouseWheelDelta > 0 && m_storedLinesToShowCount + m_newestStoredTextIndexToRender + 1 <= (int)m_storedText.size() )
				m_newestStoredTextIndexToRender++;
		}

		//Search result pulse update.
		if ( m_shouldPulseSearchResult && m_searchResultToPulse != "" )
		{
			auto storedTextIterEnd = m_storedText.end();
			for ( auto storedTextIter = m_storedText.begin(); storedTextIter != storedTextIterEnd; ++storedTextIter )
			{
				std::pair< std::string, Rgba >& currentLine = *storedTextIter;
				std::string currentLineLowercase = GetAsLowercase( currentLine.first );
				if ( currentLineLowercase.find( m_searchResultToPulse ) != std::string::npos )
				{
					currentLine.second.alphaOpacity = static_cast<unsigned char>( RangeMap( m_caretAlphaCounter, 0.f, 1.f, 0.f, 255.f ) );
				}
			}
		}
	}

}


//--------------------------------------------------------------------------------------------------------------
void TheConsole::UpdatePromptForKeydown( unsigned char ch )
{
	switch ( ch )
	{
	//Cursor control.
	case VK_LEFT:
		if ( m_caretPosInInputString - 1 >= 0 )
			m_caretPosInInputString--;
		break;
	case VK_RIGHT:
		if ( m_caretPosInInputString + 1 <= (int)m_currentPromptString.size() ) //Max valid value is actually .size(), not .size()-1.
			m_caretPosInInputString++;
		break;
	case VK_DELETE:
		if ( m_currentPromptString.size() > 0 )
		{
			m_currentPromptString.erase( m_currentPromptString.begin() + m_caretPosInInputString );
		}
		break;
	case VK_HOME:
		m_caretPosInInputString = 0;
		break;
	case VK_END:
		m_caretPosInInputString = m_currentPromptString.size();
		break;

	//Command history.
	case VK_UP: //Up is older, or further back (--) in container.
		if ( m_replacerPos - 1 >= 0 )
		{
			m_replacerPos--;
			m_currentPromptString = m_storedText[ m_replacerPos ].first;
			m_caretPosInInputString = m_currentPromptString.size();
		}
		else
		{
			if ( m_storedText.size() > 0 )
			{
				m_currentPromptString = m_storedText[ m_replacerPos ].first;
				m_caretPosInInputString = m_currentPromptString.size();
			}
		}
		break;
	case VK_DOWN:
		if ( m_replacerPos + 1 < (int)m_storedText.size() )
		{
			m_replacerPos++;
			m_currentPromptString = m_storedText[ m_replacerPos ].first;
			m_caretPosInInputString = m_currentPromptString.size();
		}
		else
		{
			if ( m_storedText.size() > 0 )
			{
				m_currentPromptString = m_storedText[ m_replacerPos ].first;
				m_caretPosInInputString = m_currentPromptString.size();
			}
		}

	//Scrollable history.
	case VK_PAGEDOWN:
		LowerShownLines();
		break;
	case VK_PAGEUP:
		RaiseShownLines();
		break;
	}
}


//--------------------------------------------------------------------------------------------------------------
void TheConsole::LowerShownLines()
{
	if ( m_newestStoredTextIndexToRender - 1 >= 0 )
		m_newestStoredTextIndexToRender--;
}


//--------------------------------------------------------------------------------------------------------------
void TheConsole::RaiseShownLines()
{
	if ( m_storedLinesToShowCount + m_newestStoredTextIndexToRender + 1 <= (int)m_storedText.size() )
		m_newestStoredTextIndexToRender++;
}


//--------------------------------------------------------------------------------------------------------------
void TheConsole::ShouldPulseSearchResults( bool newVal, const std::string& term )
{
	if ( m_searchResultToPulse != "" &&  m_searchResultToPulse != term )
	{
		//Cleanup transparencies.
		auto storedTextIterEnd = m_storedText.end();
		for ( auto storedTextIter = m_storedText.begin(); storedTextIter != storedTextIterEnd; ++storedTextIter )
		{
			std::pair< std::string, Rgba >& currentLine = *storedTextIter;
			std::string currentLineLowercase = GetAsLowercase( currentLine.first );
			if ( currentLineLowercase.find( m_searchResultToPulse ) != std::string::npos )
			{
				currentLine.second.alphaOpacity = 255;
			}
		}
	}

	m_shouldPulseSearchResult = newVal; 
	m_searchResultToPulse = GetAsLowercase( term );
}


//--------------------------------------------------------------------------------------------------------------
void TheConsole::UpdatePromptForChar( unsigned char ch )
{
	unsigned int indexOfNewestStoredText = ( m_storedText.size() > 0 ) ? m_storedText.size() - 1 : 0;
	switch ( ch )
	{
		case VK_ENTER: 
			if ( m_currentPromptString.size() <= 0 )
				HideConsole();
			else
			{
				Printf( m_currentPromptString.c_str() );
				RunCommand( m_currentPromptString );
			}
			m_replacerPos = indexOfNewestStoredText;
			m_currentPromptString = "";
			m_caretPosInInputString = 0;
			m_newestStoredTextIndexToRender = indexOfNewestStoredText;
			break;
		case VK_ESCAPE:
			if ( m_currentPromptString.size() <= 0 )
				HideConsole();
			m_currentPromptString = "";
			m_caretPosInInputString = 0;
			break;
		case VK_BACKSPACE:
			if ( m_currentPromptString.size() > 0 && m_caretPosInInputString > 0 )
			{
				m_currentPromptString.erase( m_currentPromptString.begin() + m_caretPosInInputString - 1 );
				m_caretPosInInputString--;
			}
			break;
		case '\t': //Could also do VK_TAB in UpdateForKeydown, but that'd make caret weird as '\t' would still trigger default case below.
			AttemptAutocomplete();
			break;
		default: 
			m_currentPromptString.insert( m_currentPromptString.begin() + m_caretPosInInputString, ch );
			m_caretPosInInputString++;
			break;
	}	
}


//--------------------------------------------------------------------------------------------------------------
void TheConsole::AttemptAutocomplete( const std::map< std::string, ConsoleCommandCallback* >::const_iterator* lastResultIter /*= nullptr*/ ) //Used to skip to next command.
{
	const std::string originalLastAutocompleteInput = m_lastAutocompleteInput; //Each recursion iteration will have its own, no fear of overwriting.
	unsigned int consoleIndex = 0; //Can't index into a map but by keys, yet we need index below nonetheless.
	auto candidateIterEnd = s_theConsoleCommands.cend();
	auto candidateIterStart = ( lastResultIter == nullptr ) ? s_theConsoleCommands.cbegin() : *lastResultIter; //Begin after the last result.
	for ( auto candidateIter = candidateIterStart; candidateIter != candidateIterEnd; ++candidateIter, ++consoleIndex )
	{
		const std::string& candidateName = candidateIter->first;
		if ( candidateName == m_currentPromptString )
		{
			//Because full the command matches, presume user wants the next.

			m_currentPromptString = m_lastAutocompleteInput;

			m_caretPosInInputString = m_currentPromptString.size();

			AttemptAutocomplete( &candidateIter ); //Below branch is the exit case. Iter to 'clear' being passed in here for below example!

			return;
		}

		//Otherwise check registered commands against partially entered term.
		const std::string candidateNameSubstring = candidateName.substr( 0, m_currentPromptString.size() );
		if ( candidateNameSubstring == m_currentPromptString ) //Case to test: grab to a command between two others, does it ever wrap around on its own?
		{
			//clear
			//clear2 -- upon recursive iteration for this, just goes back to setting it to clear again.
			//close

			if ( lastResultIter  )

			if ( lastResultIter != nullptr && candidateIter == *lastResultIter ) //'clear' is lastResultIter in the problem example.
				continue;

			m_lastAutocompleteInput = m_currentPromptString;

			m_currentPromptString = candidateName;

			m_caretPosInInputString = m_currentPromptString.size();

			return;
		}
	}
}