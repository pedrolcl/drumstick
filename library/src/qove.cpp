/*
    Overture OVE File component
    Copyright (C) 2006-2010, Rui Fan <vanferry@gmail.com>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "qove.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <string.h>

#include <QFile>
#include <QIODevice>
#include <QMap>
#include <QPair>
#include <QTextCodec>
#include <QString>
#include <QStringList>

#ifdef WIN32
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

namespace OVE {

class OveSong;
class Track;
class Page;
class Voice;
class Line;
class Staff;
class Measure;
class MeasureData;
class MusicData;
class OffsetElement;
class LineElement;
class PairEnds;
class Note;
class NoteContainer;
class Beam;
class Tie;
class Tuplet;
class Harmony;
class Clef;
class Lyric;
class Slur;
class MeasureText;
class Articulation;
class Glissando;
class Decorator;
class MeasureRepeat;
class Dynamics;
class Wedge;
class WedgeEndPoint;
class Pedal;
class KuoHao;
class Expressions;
class HarpPedal;
class MultiMeasureRest;
class OctaveShift;
class OctaveShiftEndPoint;
class BarNumber;
class Tempo;
class Text;
class TimeSignature;
class Key;
class RepeatSymbol;
class NumericEnding;
class MidiData;
class MidiController;
class MidiProgramChange;
class MidiChannelPressure;
class MidiPitchWheel;

const int TWELVE_TONE = 12 ;
const int INVALID_NOTE = -1 ;
const int OCTAVE = 7 ;

enum CondType {
	Cond_Time_Parameters	= 0x09,	// size - 7, TimeSignature
	Cond_Bar_Number			= 0x0A,	// size, compatible with previous version
	Cond_Decorator			= 0x16,
	Cond_Tempo				= 0x1C,	// size - 7
	Cond_Text				= 0x1D,	// size - 7, Rehearsal | SystemText
	Cond_Expression			= 0x25,	// size - 7, if set playback parameters in Expression, will store in COND
	Cond_Barline_Parameters	= 0x30, // size - 7, include :|| repeat count
	Cond_Repeat				= 0x31,	//
	Cond_Numeric_Ending		= 0x32, //

	Cond_None
};

enum BdatType {
	Bdat_Raw_Note				= 0x70,
	Bdat_Rest					= 0x80,
	Bdat_Note					= 0x90,
	Bdat_Beam					= 0x10,
	Bdat_Harmony				= 0x11,
	Bdat_Clef					= 0x12,
	Bdat_Dynamics				= 0x13,
	Bdat_Wedge					= 0x14,	// cresendo, decresendo
	Bdat_Glissando				= 0x15,
	Bdat_Decorator				= 0x16,	// measure repeat | piano pedal | dotted barline
	Bdat_Key					= 0x17,
	Bdat_Lyric					= 0x18,
	Bdat_Octave_Shift			= 0x19,
	Bdat_Slur					= 0x1B,
	Bdat_Text					= 0x1D,
	Bdat_Tie					= 0x1E,
	Bdat_Tuplet					= 0x1F,
	Bdat_Guitar_Bend			= 0x21,	//
	Bdat_Guitar_Barre			= 0x22,	//
	Bdat_Pedal					= 0x23,
	Bdat_KuoHao					= 0x24,	// () [] {}
	Bdat_Expressions			= 0x25,
	Bdat_Harp_Pedal				= 0x26,
	Bdat_Multi_Measure_Rest		= 0x27,
	Bdat_Harmony_GuitarFrame	= 0x28,
	Bdat_Graphics_40			= 0x40,	// unknown
	Bdat_Graphics_RoundRect		= 0x41,
	Bdat_Graphics_Rect			= 0x42,
	Bdat_Graphics_Round			= 0x43,
	Bdat_Graphics_Line			= 0x44,
	Bdat_Graphics_Curve			= 0x45,
	Bdat_Graphics_WedgeSymbol	= 0x46,
	Bdat_Midi_Controller		= 0xAB,
	Bdat_Midi_Program_Change	= 0xAC,
	Bdat_Midi_Channel_Pressure	= 0xAD,
	Bdat_Midi_Pitch_Wheel		= 0xAE,
	Bdat_Bar_End				= 0xFF,

	Bdat_None
};

////////////////////////////////////////

enum MusicDataType {
	// attributes
	MusicData_Clef,
	MusicData_Key,
	MusicData_Measure_Repeat,

	// sound
	MusicData_Tempo,

	// direction
	MusicData_Dynamics,
	MusicData_Wedge,
	MusicData_Wedge_EndPoint,
	MusicData_OctaveShift,
	MusicData_OctaveShift_EndPoint,
	MusicData_Expressions,
	MusicData_Repeat,
	MusicData_Text,
	MusicData_Harp_Pedal,
	MusicData_Pedal,

	// note & harmony
	MusicData_Note_Container,
	MusicData_Harmony,

	// note's children
	MusicData_Beam,
	MusicData_Glissando,
	MusicData_Lyric,
	MusicData_Slur,
	MusicData_Tie,
	MusicData_Tuplet,

	// barline
	MusicData_Numeric_Ending,

	MusicData_KuoHao,
	MusicData_Bar_End,
	MusicData_Decorator,
	MusicData_Multi_Measure_Rest,

	MusicData_None
};

enum MidiType {
	Midi_Controller = 0,
	Midi_Program_Change,
	Midi_Channel_Pressure,
	Midi_Pitch_Wheel,

	Midi_None
};

enum ClefType {
	Clef_Treble = 0x00,	//0x00
	Clef_Bass,			//0x01
	Clef_Alto,			//0x02
	Clef_UpAlto,		//0x03
	Clef_DownDownAlto,	//0x04
	Clef_DownAlto,		//0x05
	Clef_UpUpAlto,		//0x06
	Clef_Treble8va,		//0x07
	Clef_Bass8va,		//0x08
	Clef_Treble8vb,		//0x09
	Clef_Bass8vb,		//0x0A
	Clef_Percussion1,	//0x0B
	Clef_Percussion2,	//0x0C
	Clef_TAB			//0x0D
};

enum GroupType {
	Group_None = 0,
	Group_Brace,
	Group_Bracket
};

enum AccidentalType {
	Accidental_Normal				= 0x0,
	Accidental_Sharp				= 0x1,
	Accidental_Flat					= 0x2,
	Accidental_Natural				= 0x3,
	Accidental_DoubleSharp			= 0x4,
	Accidental_DoubleFlat			= 0x5,
	Accidental_Sharp_Caution		= 0x9,
	Accidental_Flat_Caution			= 0xA,
	Accidental_Natural_Caution		= 0xB,
	Accidental_DoubleSharp_Caution	= 0xC,
	Accidental_DoubleFlat_Caution	= 0xD
};

enum NoteHeadType {
	NoteHead_Standard	= 0x00,
	NoteHead_Invisible,
	NoteHead_Rhythmic_Slash,
	NoteHead_Percussion,
	NoteHead_Closed_Rhythm,
	NoteHead_Open_Rhythm,
	NoteHead_Closed_Slash,
	NoteHead_Open_Slash,
	NoteHead_Closed_Do,
	NoteHead_Open_Do,
	NoteHead_Closed_Re,
	NoteHead_Open_Re,
	NoteHead_Closed_Mi,
	NoteHead_Open_Mi,
	NoteHead_Closed_Fa,
	NoteHead_Open_Fa,
	NoteHead_Closed_Sol,
	NoteHead_Open_Sol,
	NoteHead_Closed_La,
	NoteHead_Open_La,
	NoteHead_Closed_Ti,
	NoteHead_Open_Ti
};

enum TiePos {
	Tie_None		= 0x0,
	Tie_LeftEnd		= 0x1,
	Tie_RightEnd	= 0x2
};

enum ArticulationType {
	Articulation_Major_Trill			= 0x00,
	Articulation_Minor_Trill			= 0x01,
	Articulation_Trill_Section			= 0x02,
	Articulation_Inverted_Short_Mordent	= 0x03,
	Articulation_Inverted_Long_Mordent	= 0x04,
	Articulation_Short_Mordent			= 0x05,
	Articulation_Turn					= 0x06,
	Articulation_Finger_1				= 0x07,
	Articulation_Finger_2				= 0x08,
	Articulation_Finger_3				= 0x09,
	Articulation_Finger_4				= 0x0A,
	Articulation_Finger_5				= 0x0B,
	Articulation_Flat_Accidental_For_Trill = 0x0C,
	Articulation_Sharp_Accidental_For_Trill = 0x0D,
	Articulation_Natural_Accidental_For_Trill = 0x0E,
	Articulation_Marcato				= 0x0F,
	Articulation_Marcato_Dot			= 0x10,
	Articulation_Heavy_Attack			= 0x11,
	Articulation_SForzando				= 0x12,
	Articulation_SForzando_Dot			= 0x13,
	Articulation_Heavier_Attack			= 0x14,
	Articulation_SForzando_Inverted		= 0x15,
	Articulation_SForzando_Dot_Inverted	= 0x16,
	Articulation_Staccatissimo			= 0x17,
	Articulation_Staccato				= 0x18,
	Articulation_Tenuto					= 0x19,
	Articulation_Up_Bow					= 0x1A,
	Articulation_Down_Bow				= 0x1B,
	Articulation_Up_Bow_Inverted		= 0x1C,
	Articulation_Down_Bow_Inverted		= 0x1D,
	Articulation_Arpeggio				= 0x1E,
	Articulation_Tremolo_Eighth			= 0x1F,
	Articulation_Tremolo_Sixteenth		= 0x20,
	Articulation_Tremolo_Thirty_Second	= 0x21,
	Articulation_Tremolo_Sixty_Fourth	= 0x22,
	Articulation_Natural_Harmonic		= 0x23,
	Articulation_Artificial_Harmonic	= 0x24,
	Articulation_Plus_Sign				= 0x25,
	Articulation_Fermata				= 0x26,
	Articulation_Fermata_Inverted		= 0x27,
	Articulation_Pedal_Down				= 0x28,
	Articulation_Pedal_Up				= 0x29,
	Articulation_Pause					= 0x2A,
	Articulation_Grand_Pause			= 0x2B,
	Articulation_Toe_Pedal				= 0x2C,
	Articulation_Heel_Pedal				= 0x2D,
	Articulation_Toe_To_Heel_Pedal		= 0x2E,
	Articulation_Heel_To_Toe_Pedal		= 0x2F,
	Articulation_Open_String			= 0x30,	// finger 0 in guitar
	Articulation_Guitar_Lift			= 0x46,
	Articulation_Guitar_Slide_Up		= 0x47,
	Articulation_Guitar_Rip				= 0x48,
	Articulation_Guitar_Fall_Off		= 0x49,
	Articulation_Guitar_Slide_Down		= 0x4A,
	Articulation_Guitar_Spill			= 0x4B,
	Articulation_Guitar_Flip			= 0x4C,
	Articulation_Guitar_Smear			= 0x4D,
	Articulation_Guitar_Bend			= 0x4E,
	Articulation_Guitar_Doit			= 0x4F,
	Articulation_Guitar_Plop			= 0x50,
	Articulation_Guitar_Wow_Wow			= 0x51,
	Articulation_Guitar_Thumb			= 0x64,
	Articulation_Guitar_Index_Finger	= 0x65,
	Articulation_Guitar_Middle_Finger	= 0x66,
	Articulation_Guitar_Ring_Finger		= 0x67,
	Articulation_Guitar_Pinky_Finger	= 0x68,
	Articulation_Guitar_Tap				= 0x69,
	Articulation_Guitar_Hammer			= 0x6A,
	Articulation_Guitar_Pluck			= 0x6B,

	Articulation_None

/*	Articulation_Detached_Legato,
	Articulation_Spiccato,
	Articulation_Scoop,
	Articulation_Plop,
	Articulation_Doit,
	Articulation_Falloff,
	Articulation_Breath_Mark,
	Articulation_Caesura,*/
};

enum NoteType {
	Note_DoubleWhole= 0x0,
	Note_Whole		= 0x1,
	Note_Half		= 0x2,
	Note_Quarter	= 0x3,
	Note_Eight		= 0x4,
	Note_Sixteen	= 0x5,
	Note_32			= 0x6,
	Note_64			= 0x7,
	Note_128		= 0x8,
	Note_256		= 0x9,

	Note_None
};

inline int NoteTypeToTick(NoteType type, int quarter) {
	int c = int(pow(2.0, (int)type)) ;
	return quarter * 4 * 2 / c ;
}

enum HarmonyType {
	Harmony_maj = 0,
	Harmony_min,
	Harmony_aug,
	Harmony_dim,
	Harmony_dim7,
	Harmony_sus2,
	Harmony_sus4,
	Harmony_sus24,
	Harmony_add2,
	Harmony_add9,
	Harmony_omit3,
	Harmony_omit5,
	Harmony_2,
	Harmony_5,
	Harmony_6,
	Harmony_69,
	Harmony_7,
	Harmony_7b5,
	Harmony_7b9,
	Harmony_7s9,
	Harmony_7s11,
	Harmony_7b5s9,
	Harmony_7b5b9,
	Harmony_7b9s9,
	Harmony_7b9s11,
	Harmony_7sus4,
	Harmony_9,
	Harmony_9b5,
	Harmony_9s11,
	Harmony_9sus4,
	Harmony_11,
	Harmony_13,
	Harmony_13b5,
	Harmony_13b9,
	Harmony_13s9,
	Harmony_13s11,
	Harmony_13sus4,
	Harmony_min_add2,
	Harmony_min_add9,
	Harmony_min_maj7,
	Harmony_min6,
	Harmony_min6_add9,
	Harmony_min7,
	Harmony_min7b5,
	Harmony_min7_add4,
	Harmony_min7_add11,
	Harmony_min9,
	Harmony_min9_b5,
	Harmony_min9_maj7,
	Harmony_min11,
	Harmony_min13,
	Harmony_maj7,
	Harmony_maj7_b5,
	Harmony_maj7_s5,
	Harmony_maj7_69,
	Harmony_maj7_add9,
	Harmony_maj7_s11,
	Harmony_maj9,
	Harmony_maj9_sus4,
	Harmony_maj9_b5,
	Harmony_maj9_s5,
	Harmony_maj9_s11,
	Harmony_maj13,
	Harmony_maj13_b5,
	Harmony_maj13_b9,
	Harmony_maj13_b9b5,
	Harmony_maj13_s11,
	Harmony_aug7,
	Harmony_aug7_b9,
	Harmony_aug7_s9,

	Harmony_None
};

enum DynamicsType {
	Dynamics_pppp = 0,
	Dynamics_ppp,
	Dynamics_pp,
	Dynamics_p,
	Dynamics_mp,
	Dynamics_mf,
	Dynamics_f,
	Dynamics_ff,
	Dynamics_fff,
	Dynamics_ffff,
	Dynamics_sf,
	Dynamics_fz,
	Dynamics_sfz,
	Dynamics_sffz,
	Dynamics_fp,
	Dynamics_sfp
};

enum WedgeType {
	Wedge_Cres_Line = 0,	// <
	Wedge_Double_Line,		// <>, not appear in xml
	Wedge_Decresc_Line,		// >
	Wedge_Cres,				// cresc., not appear in xml, will create Expression
	Wedge_Decresc			// decresc., not appear in xml, will create Expression
};

enum KuoHaoType {
	KuoHao_Parentheses = 0,
	KuoHao_Brace,
	KuoHao_Bracket
};

enum OctaveShiftType {
	OctaveShift_8 = 0,
	OctaveShift_Minus_8,
	OctaveShift_15,
	OctaveShift_Minus_15
};

enum OctaveShiftPosition {
	OctavePosition_Start = 0 ,
	OctavePosition_Continue,
	OctavePosition_Stop
};

enum RepeatType {
	Repeat_Segno = 0,
	Repeat_Coda,
	Repeat_ToCoda,
	Repeat_DSAlCoda,
	Repeat_DSAlFine,
	Repeat_DCAlCoda,
	Repeat_DCAlFine,
	Repeat_Fine,

	Repeat_Null
};

enum BarlineType {
	Barline_Default = 0,	//0x00 will be | or final (at last measure)
	Barline_Double,			//0x01 ||
	Barline_RepeatLeft,		//0x02 ||:
	Barline_RepeatRight,	//0x03 :||
	Barline_Final,			//0x04
	Barline_Dashed,			//0x05
	Barline_Null			//0x06
} ;

enum NoteDuration {
	NoteDuration_256 = 15,
	NoteDuration_128 = NoteDuration_256 * 2,			// 30
	NoteDuration_64 = NoteDuration_128 * 2,				// 60
	NoteDuration_32 = NoteDuration_64 * 2,				// 120
	NoteDuration_16 = NoteDuration_32 * 2,				// 240
	NoteDuration_8 = NoteDuration_16 * 2,				// 480
	NoteDuration_4 = NoteDuration_8 * 2,				// 960
	NoteDuration_2 = NoteDuration_4 * 2,				// 1920
	NoteDuration_Whole = NoteDuration_2 * 2,			// 3840
	NoteDuration_Double_Whole = NoteDuration_Whole * 2	// 7680
};

enum ToneType {
	Tone_C = 0 ,
	Tone_D,
	Tone_E,
	Tone_F,
	Tone_G,
	Tone_A,
	Tone_B
};

enum KeyType {
	Key_C = 0,		// C
	Key_Bass_1,		// F
	Key_Bass_2,		// Bb
	Key_Bass_3,		// Eb
	Key_Bass_4,		// Ab
	Key_Bass_5,		// Db
	Key_Bass_6,		// Gb
	Key_Bass_7,		// Cb
	Key_Sharp_1,	// G
	Key_Sharp_2,	// D
	Key_Sharp_3,	// A
	Key_Sharp_4,	// E
	Key_Sharp_5,	// B
	Key_Sharp_6,	// F#
	Key_Sharp_7		// C#
};

// IOveNotify.h
class IOveNotify {
public:
	IOveNotify() {}
	virtual ~IOveNotify() {}

public:
	virtual void loadInfo(const QString& info) = 0;
	virtual void loadError() = 0;
	virtual void loadPosition(int currentMeasure, int totalMeasure, int currentTrack, int totalTrack) = 0;
};

class IOVEStreamLoader {
public:
	IOVEStreamLoader() {}
	virtual ~IOVEStreamLoader() {}

public:
	virtual void setNotify(IOveNotify* notify) = 0;
	virtual void setFileStream(unsigned char* buffer, unsigned int size) = 0;
	virtual void setOve(OveSong* ove) = 0;

	// read stream, set readed data to setOve(ove)
	virtual bool load() = 0;

	virtual void release() = 0;
};

DLL_EXPORT IOVEStreamLoader* createOveStreamLoader();

/////////////////////////////////////////////////////////////////////////////
// basic element
class TickElement {
public:
	TickElement();
	virtual ~TickElement() {}

public:
	void setTick(int tick);
	int getTick(void) const;

private:
	int tick_;
};

class MeasurePos {
public:
	MeasurePos();
	virtual ~MeasurePos() {}

public:
	void setMeasure(int measure);
	int getMeasure() const;

	void setOffset(int offset);
	int getOffset() const;

	MeasurePos shiftMeasure(int measure) const;
	MeasurePos shiftOffset(int offset) const; // ignore cross measure

	bool operator ==(const MeasurePos& mp) const;
	bool operator !=(const MeasurePos& mp) const;
	bool operator <(const MeasurePos& mp) const;
	bool operator <=(const MeasurePos& mp) const;
	bool operator >(const MeasurePos& mp) const;
	bool operator >=(const MeasurePos& mp) const;

private:
	int measure_;
	int offset_;
};

class PairElement {
public:
	PairElement();
	virtual ~PairElement();

public:
	MeasurePos* start() const;
	MeasurePos* stop() const;

private:
	MeasurePos* start_;
	MeasurePos* stop_;
};

class PairEnds {
public:
	PairEnds();
	virtual ~PairEnds();

public:
	LineElement* getLeftLine() const;
	LineElement* getRightLine() const;

	OffsetElement* getLeftShoulder() const;
	OffsetElement* getRightShoulder() const;

private:
	LineElement* leftLine_;
	LineElement* rightLine_;
	OffsetElement* leftShoulder_;
	OffsetElement* rightShoulder_;
};

class LineElement {
public:
	LineElement();
	virtual ~LineElement() {}

public:
	virtual void setLine(int line); // middle line (3rd line of each clef) is set 0
	virtual int getLine(void) const;

private:
	int line_;
};

class OffsetElement {
public:
	OffsetElement();
	virtual ~OffsetElement() {}

public:
	virtual void setXOffset(int offset);
	virtual int getXOffset() const;

	virtual void setYOffset(int offset);
	virtual int getYOffset() const;

private:
	int xOffset_;
	int yOffset_;
};

class LengthElement {
public:
	LengthElement();
	virtual ~LengthElement() {}

public:
	void setLength(int length);
	int getLength() const;

private:
	int length_; // tick
};

// base class of many ove music element
class MusicData: public TickElement, public PairElement, public OffsetElement {
public:
	MusicData();
	virtual ~MusicData() {}

public:
	MusicDataType getMusicDataType() const;

	enum XmlDataType {
		Attributes = 0, NoteBeam, Notations, Direction, None
	};
	static XmlDataType getXmlDataType(MusicDataType type);
	//	static bool get_is_pair_element(MusicDataType type) ;

	// show / hide
	void setShow(bool show);
	bool getShow() const;

	// color
	void setColor(unsigned int color); // not exists in ove 3
	unsigned int getColor() const;

	void setVoice(unsigned int voice);
	unsigned int getVoice() const;

	void copyCommonBlock(const MusicData& source);

protected:
	MusicDataType musicDataType_;

private:
	bool show_;
	unsigned int color_;
	unsigned int voice_;
};

class MidiData: public TickElement {
public:
	MidiData();
	virtual ~MidiData() {}

public:
	MidiType getMidiType() const;

protected:
	MidiType midiType_;
};


////////////////////////////////////////////////////////////////////////////////
class OveSong {
public:
	OveSong();
	~OveSong();

public:
	void setIsVersion4(bool version4 = true);
	bool getIsVersion4() const;

	void setQuarter(int tick);
	int getQuarter(void) const;

	void setShowPageMargin(bool show);
	bool getShowPageMargin() const;

	void setShowTransposeTrack(bool show);
	bool getShowTransposeTrack() const;

	void setShowLineBreak(bool show);
	bool getShowLineBreak() const;

	void setShowRuler(bool show);
	bool getShowRuler() const;

	void setShowColor(bool show);
	bool getShowColor() const;

	void setPlayRepeat(bool play);
	bool getPlayRepeat() const;

	enum PlayStyle{
		Record,	Swing, Notation
	};
	void setPlayStyle(PlayStyle style);
	PlayStyle getPlayStyle() const;

	void addTitle(const QString& str);
	QList<QString> getTitles(void) const;

	void addAnnotate(const QString& str);
	QList<QString> getAnnotates(void) const;

	void addWriter(const QString& str);
	QList<QString> getWriters(void) const;

	void addCopyright(const QString& str);
	QList<QString> getCopyrights(void) const;

	void addHeader(const QString& str);
	QList<QString> getHeaders(void) const;

	void addFooter(const QString& str);
	QList<QString> getFooters(void) const;

	void addTrack(Track* ptr);
	int getTrackCount(void) const;
	QList<Track*> getTracks() const;
	Track* getTrack(int part, int staff) const;

	void setTrackBarCount(int count);
	int getTrackBarCount() const;

	bool addPage(Page* page);
	int getPageCount() const;
	Page* getPage(int idx);

	void addLine(Line* ptr);
	int getLineCount() const;
	Line* getLine(int idx) const;

	void addMeasure(Measure* ptr);
	int getMeasureCount(void) const;
	Measure* getMeasure(int bar) const;

	void addMeasureData(MeasureData* ptr);
	int getMeasureDataCount(void) const;
	MeasureData* getMeasureData(int part, int staff/*=0*/, int bar) const;
	MeasureData* getMeasureData(int track, int bar) const;

	// tool
	void setPartStaffCounts(const QList<int>& partStaffCounts);
	int getPartCount() const;
	int getStaffCount(int part) const;
	int getPartBarCount() const;

	void clear(void);

	QPair<int, int> trackToPartStaff(int track) const;

	void setTextCodecName(const QString& codecName);
	QString getCodecString(const QByteArray& text);

private:
	int partStaffToTrack(int part, int staff) const;

private:
	bool version4_;
	int quarter_;

	bool showPageMargin_;
	bool showTransposeTrack;
	bool showLineBreak_;
	bool showRuler_;
	bool showColor_;
	bool playRepeat_;
	PlayStyle playStyle_;

	QList<QString> titles_;
	QList<QString> annotates_;
	QList<QString> writers_;
	QList<QString> copyrights_;
	QList<QString> headers_;
	QList<QString> footers_;

	QList<Track*> tracks_;
	QList<Page*> pages_;
	QList<Line*> lines_;
	QList<Measure*> measures_;
	QList<MeasureData*> measureDatas_;
	int trackBarCount_;	//equal to measures_.size()

	QList<int> partStaffCounts_;
	QTextCodec* codec_;
};

class Voice {
public:
	Voice();
	~Voice(){}

public:
	void setChannel(int channel);
	int getChannel() const;

	void setVolume(int volume);
	int getVolume() const;

	void setPitchShift(int pitchShift);
	int getPitchShift() const;

	void setPan(int pan);
	int getPan() const;

	void setPatch(int patch);
	int getPatch() const;

	void setStemType(int stemType);
	int getStemType() const;

	static int getDefaultPatch();
	static int getDefaultVolume();

private:
	int channel_;		// [0, 15]
	int volume_;		// [-1, 127], -1 default
	int pitchShift_;	// [-36, 36]
	int pan_;			// [-64, 63]
	int patch_;			// [0, 127]
	int stemType_;		// 0, 1, 2
};

class Track {
public:
	Track();
	~Track();

public:
	void setName(const QString& str);
	QString getName(void) const;

	void setBriefName(const QString& str);
	QString getBriefName(void) const;

	void setPatch(unsigned int patch); // -1: percussion
	unsigned int getPatch() const;

	void setChannel(int channel);
	int getChannel() const;

	void setShowName(bool show);
	bool getShowName() const;

	void setShowBriefName(bool show);
	bool getShowBriefName() const;

	void setMute(bool mute);
	bool getMute() const;

	void setSolo(bool solo);
	bool getSolo() const;

	void setShowKeyEachLine(bool show);
	bool getShowKeyEachLine() const;

	void setVoiceCount(int voices);
	int getVoiceCount() const;

	void addVoice(Voice* voice);
	QList<Voice*> getVoices() const;

	void setShowTranspose(bool show);
	bool getShowTranspose() const;

	void setTranspose(int transpose);
	int getTranspose() const;

	void setNoteShift(int shift);
	int getNoteShift() const;

	void setStartClef(int clef/*in ClefType*/);
	ClefType getStartClef() const;

	void setTransposeClef(int clef);
	int getTansposeClef() const;

	void setStartKey(int key/*in KeyType*/);
	int getStartKey() const;

	void setDisplayPercent(unsigned int percent/*25~100*/);
	unsigned int getDisplayPercent() const;

	void setShowLegerLine(bool show);
	bool getShowLegerLine() const;

	void setShowClef(bool show);
	bool getShowClef() const;

	void setShowTimeSignature(bool show);
	bool getShowTimeSignature() const;

	void setShowKeySignature(bool show);
	bool getShowKeySignature() const;

	void setShowBarline(bool show);
	bool getShowBarline() const;

	void setFillWithRest(bool fill);
	bool getFillWithRest() const;

	void setFlatTail(bool flat);
	bool getFlatTail() const;

	void setShowClefEachLine(bool show);
	bool getShowClefEachLine() const;

	struct DrumNode {
		int line_;
		int headType_;
		int pitch_;
		int voice_;

	public:
		DrumNode():line_(0), headType_(0), pitch_(0), voice_(0){}
	};
	void addDrum(const DrumNode& node);
	QList<DrumNode> getDrumKit() const;

	void clear(void);

	/////////////////////////////////////////////////
	void setPart(int part);
	int getPart() const;

private:
	int number_;
	QString name_;
	QString briefName_;
	unsigned int patch_;
	int channel_;
	int transpose_;
	bool showTranspose_;
	int noteShift_;
	int startClef_;
	int transposeClef_;
	unsigned int displayPercent_;
	int startKey_;
	int voiceCount_;
	QList<Voice*> voices_;

	bool showName_;
	bool showBriefName_;
	bool showKeyEachLine_;
	bool showLegerLine_;
	bool showClef_;
	bool showTimeSignature_;
	bool showKeySignature_;
	bool showBarline_;
	bool showClefEachLine_;

	bool fillWithRest_;
	bool flatTail_;

	bool mute_;
	bool solo_;

	QList<DrumNode> drumKit_;

	//////////////////////////////
	int part_;
};

class Page {
public:
	Page();
	~Page(){}

public:
	void setBeginLine(int line);
	int getBeginLine() const;

	void setLineCount(int count);
	int getLineCount() const;

	void setLineInterval(int interval);	// between system
	int getLineInterval() const;

	void setStaffInterval(int interval);
	int getStaffInterval() const;

	void setStaffInlineInterval(int interval); // between treble-bass staff
	int getStaffInlineInterval() const;

	void setLineBarCount(int count);
	int getLineBarCount() const;

	void setPageLineCount(int count);
	int getPageLineCount() const;

	void setLeftMargin(int margin);
	int getLeftMargin() const;

	void setTopMargin(int margin);
	int getTopMargin() const;

	void setRightMargin(int margin);
	int getRightMargin() const;

	void setBottomMargin(int margin);
	int getBottomMargin() const;

	void setPageWidth(int width);
	int getPageWidth() const;

	void setPageHeight(int height);
	int getPageHeight() const;

private:
	int beginLine_;
	int lineCount_;

	int lineInterval_;
	int staffInterval_;
	int staffInlineInterval_;

	int lineBarCount_;
	int pageLineCount_;

	int leftMargin_;
	int topMargin_;
	int rightMargin_;
	int bottomMargin_;

	int pageWidth_;
	int pageHeight_;
};

class Line {
public:
	Line();
	~Line();

public:
	void addStaff(Staff* staff);
	int getStaffCount() const;
	Staff* getStaff(int idx) const;

	void setBeginBar(unsigned int bar);
	unsigned int getBeginBar() const;

	void setBarCount(unsigned int count);
	unsigned int getBarCount() const;

	void setYOffset(int offset);
	int getYOffset() const;

	void setLeftXOffset(int offset);
	int getLeftXOffset() const;

	void setRightXOffset(int offset);
	int getRightXOffset() const;

private:
	QList<Staff*> staffs_;
	unsigned int beginBar_;
	unsigned int barCount_;
	int yOffset_;
	int leftXOffset_;
	int rightXOffset_;
};

class Staff : public OffsetElement {
public:
	Staff();
	virtual ~Staff(){}

public:
	void setClefType(int clef);
	ClefType getClefType() const;

	void setKeyType(int key);
	int getKeyType() const;

	void setVisible(bool visible);
	bool setVisible() const;

	void setGroupType(GroupType type);
	GroupType getGroupType() const;

	void setGroupStaffCount(int count);
	int getGroupStaffCount() const;

private:
	ClefType clef_;
	int key_;
	bool visible_;
	GroupType groupType_;
	int groupStaffCount_;
};

///////////////////////////////////////////////////////////////////////////////

class Note : public LineElement {
public:
	Note();
	virtual ~Note(){}

public:
	void setIsRest(bool rest);
	bool getIsRest() const;

	void setNote(unsigned int note);
	unsigned int getNote() const;

	void setAccidental(int type);		//AccidentalType
	AccidentalType getAccidental() const;

	void setShowAccidental(bool show);
	bool getShowAccidental() const;

	void setOnVelocity(unsigned int velocity);
	unsigned int getOnVelocity() const;

	void setOffVelocity(unsigned int velocity);
	unsigned int getOffVelocity() const;

	void setHeadType(int type);		//NoteHeadType
	NoteHeadType getHeadType() const;

	void setTiePos(int tiePos);
	TiePos getTiePos() const;

	void setOffsetStaff(int offset);	// cross staff notes
	int getOffsetStaff() const;

	void setShow(bool show);
	bool getShow() const;

	void setOffsetTick(int offset);
	int getOffsetTick() const;

private:
	bool rest_;
	unsigned int note_;
	AccidentalType accidental_;
	bool showAccidental_;
	unsigned int onVelocity_;
	unsigned int offVelocity_;
	NoteHeadType headType_;
	TiePos tiePos_;
	int offsetStaff_;
	bool show_;
	int offsetTick_;//for playback
};

class Articulation : public OffsetElement {
public:
	Articulation();
	virtual ~Articulation(){}

public:
	void setArtType(int type);//ArticulationType
	ArticulationType getArtType() const;

	void setPlacementAbove(bool above);
	bool getPlacementAbove() const;

	// for midi
	bool willAffectNotes() const;

	static bool isTrill(ArticulationType type);

	// for xml
	enum XmlType {
		Xml_Articulation,
		Xml_Technical,
		Xml_Arpeggiate,
		Xml_Ornament,
		Xml_Fermata,
		Xml_Direction,

		Xml_Unknown
	};
	XmlType getXmlType() const;

	// sound setting
	bool getChangeSoundEffect() const;
	void setSoundEffect(int soundFrom, int soundTo);
	QPair<int, int> getSoundEffect() const;

	bool getChangeLength() const;
	void setLengthPercentage(int percentage);
	int getLengthPercentage() const;

	bool getChangeVelocity() const;
	enum VelocityType
	{
		Velocity_Offset,
		Velocity_SetValue,
		Velocity_Percentage
	};
	void setVelocityType(VelocityType type);
	VelocityType getVelocityType() const;

	void setVelocityValue(int value);
	int getVelocityValue() const;

	bool getChangeExtraLength() const;
	void setExtraLength(int length);
	int getExtraLength() const;

	// trill
	enum TrillInterval {
		TrillInterval_Diatonic = 0,
		TrillInterval_Chromatic,
		TrillInterval_Whole
	};
	void setTrillInterval(int interval);
	TrillInterval getTrillInterval() const;

	void setAuxiliaryFirst(bool first);
	bool getAuxiliaryFirst() const;

	void setTrillRate(NoteType rate);
	NoteType getTrillRate() const;

	void setTrillNoteLength(int length);
	int getTrillNoteLength() const;

	enum AccelerateType {
		Accelerate_None = 0 ,
		Accelerate_Slow,
		Accelerate_Normal,
		Accelerate_Fast
	};
	void setAccelerateType(int type);
	AccelerateType getAccelerateType() const;

private:
	ArticulationType type_;
	bool above_;

	bool changeSoundEffect_;
	QPair<int, int> soundEffect_;
	bool changeLength_;
	int lengthPercentage_;
	bool changeVelocity_;
	VelocityType velocityType_;
	int velocityValue_;
	bool changeExtraLength_;
	int extraLength_;

	// trill
	TrillInterval trillInterval_;
	bool auxiliaryFirst_;
	NoteType trillRate_;
	int trillNoteLength_;
	AccelerateType accelerateType_;
};

class NoteContainer : public MusicData, public LengthElement {
public:
	NoteContainer();
	virtual ~NoteContainer();

public:
	void setIsGrace(bool grace);
	bool getIsGrace() const;

	void setIsCue(bool cue);
	bool getIsCue() const;

	void setIsRest(bool rest/*or note*/);
	bool getIsRest() const;

	void setIsRaw(bool raw);
	bool getIsRaw() const;

	void setNoteType(NoteType type);
	NoteType getNoteType() const;

	void setDot(int dot);
	int getDot() const;

	void setGraceNoteType(NoteType type);
	NoteType getGraceNoteType() const;

	void setInBeam(bool in);
	bool getInBeam() const;

	void setStemUp(bool up);
	bool getStemUp(void) const;

	void setShowStem(bool show);
	bool getShowStem() const;

	void setStemLength(int line);
	int getStemLength() const;

	void setTuplet(int tuplet);
	int getTuplet() const;

	void setSpace(int space);
	int getSpace() const;

	void addNoteRest(Note* note);
	QList<Note*> getNotesRests() const;

	void addArticulation(Articulation* art);
	QList<Articulation*> getArticulations() const;

	void setNoteShift(int octave);
	int getNoteShift() const;

	int getOffsetStaff() const;

	int getDuration() const;

private:
	bool grace_;
	bool cue_;
	bool rest_;
	bool raw_;
	NoteType noteType_;
	int dot_;
	NoteType graceNoteType_;
	int tuplet_;
	int space_;
	bool inBeam_;
	bool stemUp_;
	bool showStem_;
	int stemLength_;	// line count span
	int noteShift_;

	QList<Note*> notes_;
	QList<Articulation*> articulations_;
};

class Beam : public MusicData, public PairEnds {
public:
	Beam();
	virtual ~Beam(){}

public:
	void setIsGrace(bool grace);
	bool getIsGrace() const;

	void addLine(const MeasurePos& startMp, const MeasurePos& endMp);
	const QList<QPair<MeasurePos, MeasurePos> > getLines() const;

private:
	bool grace_;
	QList<QPair<MeasurePos, MeasurePos> > lines_;
};

class Tie : public MusicData, public PairEnds {
public:
	Tie();
	virtual ~Tie(){}

public:
	void setShowOnTop(bool top);
	bool getShowOnTop() const;

	void setNote(int note);// note value tie point to
	int getNote() const;

	void setHeight(int height);
	int getHeight() const;

private:
	bool showOnTop_;
	int note_;
	int height_;
};

class Glissando : public MusicData, public PairEnds {
public:
	Glissando();
	virtual ~Glissando(){}

public:
	void setStraightWavy(bool straight);
	bool getStraightWavy() const;

	void setText(const QString& text);
	QString getText() const;

	void setLineThick(int thick);
	int getLineThick() const;

private:
	bool straight_;
	QString text_;
	int lineThick_;
};

class Decorator : public MusicData {
public:
	Decorator();
	virtual ~Decorator(){}

public:
	enum DecoratorType {
		Decorator_Dotted_Barline = 0,
		Decorator_Articulation
	};
	void setDecoratorType(DecoratorType type);
	DecoratorType getDecoratorType() const;

	void setArticulationType(ArticulationType type);
	ArticulationType getArticulationType() const;

private:
	DecoratorType decoratorType_;
	ArticulationType artType_;
};

class MeasureRepeat : public MusicData {
public:
	MeasureRepeat();
	virtual ~MeasureRepeat(){}

public:
	void setSingleRepeat(bool single); // false : double
	bool getSingleRepeat() const;

private:
	bool singleRepeat_;
};

class Tuplet : public MusicData, public PairEnds {
public:
	Tuplet();
	virtual ~Tuplet();

public:
	void setTuplet(int tuplet=3);
	int getTuplet() const;

	void setSpace(int space=2);
	int getSpace() const;

	void setHeight(int height);
	int getHeight() const;

	void setNoteType(NoteType type);
	NoteType getNoteType() const;

	OffsetElement* getMarkHandle() const;

private:
	int tuplet_;
	int space_;
	int height_;
	NoteType noteType_;
	OffsetElement* mark_;
};

class Harmony : public MusicData, public LengthElement {
public:
	Harmony();
	virtual ~Harmony(){}

public:
	void setHarmonyType(HarmonyType type);
	HarmonyType getHarmonyType() const;

	void setRoot(int root=0);//C
	int getRoot() const;

	void setBass(int bass);
	int getBass() const;

	void setBassOnBottom(bool on);
	bool getBassOnBottom() const;

	void setAngle(int angle);
	int getAngle() const;

private:
	HarmonyType harmonyType_;
	int root_;
	int bass_;
	bool bassOnBottom_;
	int angle_;
};

class Clef : public MusicData, public LineElement {
public:
	Clef();
	virtual ~Clef(){}

public:
	void setClefType(int type);	// ClefType
	ClefType getClefType() const;

private:
	ClefType clefType_;
};

class Lyric : public MusicData {
public:
	Lyric();
	virtual ~Lyric(){}

public:
	void setLyric(const QString& lyricText);
	QString getLyric() const;

	void setVerse(int verse);
	int getVerse() const;

private:
	QString lyric_;
	int verse_;
};

class Slur: public MusicData, public PairEnds {
public:
	Slur();
	virtual ~Slur();

public:
	void setContainerCount(int count); // span
	int getContainerCount() const;

	void setShowOnTop(bool top);
	bool getShowOnTop() const;

	OffsetElement* getHandle2() const;
	OffsetElement* getHandle3() const;

	void setNoteTimePercent(int percent); // 50% ~ 200%
	int getNoteTimePercent() const;

private:
	int containerCount_;
	bool showOnTop_;
	int noteTimePercent_;
	OffsetElement* handle_2_;
	OffsetElement* handle_3_;
};

class Dynamics: public MusicData {
public:
	Dynamics();
	virtual ~Dynamics() {}

public:
	void setDynamicsType(int type);//DynamicsType
	DynamicsType getDynamicsType() const;

	void setIsPlayback(bool play);
	bool getIsPlayback() const;

	void setVelocity(int vel);
	int getVelocity() const;

private:
	DynamicsType dynamicsType_;
	bool playback_;
	int velocity_;
};

class WedgeEndPoint: public MusicData {
public:
	WedgeEndPoint();
	virtual ~WedgeEndPoint() {}

public:
	void setWedgeType(WedgeType type);
	WedgeType getWedgeType() const;

	void setHeight(int height);
	int getHeight() const;

	void setWedgeStart(bool wedgeStart);
	bool getWedgeStart() const;

private:
	int height_;
	WedgeType wedgeType_;
	bool wedgeStart_;
};

class Wedge: public MusicData {
public:
	Wedge();
	virtual ~Wedge() {}

public:
	void setWedgeType(WedgeType type);
	WedgeType getWedgeType() const;

	void setHeight(int height);
	int getHeight() const;

private:
	int height_;
	WedgeType wedgeType_;
};

class Pedal: public MusicData, public PairEnds {
public:
	Pedal();
	virtual ~Pedal();

public:
	void setHalf(bool half);
	bool getHalf() const;

	void setIsPlayback(bool playback);
	bool getIsPlayback() const;

	void setPlayOffset(int offset); // -127~127
	int getPlayOffset() const;

	OffsetElement* getPedalHandle() const; //only on half pedal

private:
	bool half_;
	bool playback_;
	int playOffset_;
	OffsetElement* pedalHandle_;
};

class KuoHao: public MusicData, public PairEnds {
public:
	KuoHao();
	virtual ~KuoHao() {}

public:
	void setHeight(int height);
	int getHeight() const;

	void setKuohaoType(int type);// KuoHaoType
	KuoHaoType getKuohaoType() const;

private:
	int height_;
	KuoHaoType kuohaoType_;
};

class Expressions: public MusicData {
public:
	Expressions();
	virtual ~Expressions() {}

public:
	void setText(const QString& str);
	QString getText() const;

private:
	QString text_;
};

class HarpPedal: public MusicData {
public:
	HarpPedal();
	virtual ~HarpPedal() {}

public:
	void setShowType(int type);//0:graph, 1:char, 2:char cut, 3:change
	int getShowType() const;

	void setShowCharFlag(int flag);//each bit is a bool, total 7 bools
	int getShowCharFlag() const;

private:
	int showType_;
	int showCharFlag_;
};

class OctaveShift: public MusicData, public LengthElement {
public:
	OctaveShift();
	virtual ~OctaveShift() {}

public:
	void setOctaveShiftType(int type);
	OctaveShiftType getOctaveShiftType() const;

	int getNoteShift() const;

	void setEndTick(int tick);
	int getEndTick() const;

private:
	OctaveShiftType octaveShiftType_;
	OctaveShiftPosition octaveShiftPosition_;
	int endTick_;
};

class OctaveShiftEndPoint: public MusicData, public LengthElement {
public:
	OctaveShiftEndPoint();
	virtual ~OctaveShiftEndPoint() {}

public:
	void setOctaveShiftType(int type);
	OctaveShiftType getOctaveShiftType() const;

	void setOctaveShiftPosition(int position);
	OctaveShiftPosition getOctaveShiftPosition() const;

	void setEndTick(int tick);
	int getEndTick() const;

private:
	OctaveShiftType octaveShiftType_;
	OctaveShiftPosition octaveShiftPosition_;
	int endTick_;
};

class MultiMeasureRest: public MusicData {
public:
	MultiMeasureRest();
	virtual ~MultiMeasureRest() {}

public:
	void setMeasureCount(int count);
	int getMeasureCount() const;

private:
	int measureCount_;
};

class Tempo: public MusicData {
public:
	Tempo();
	virtual ~Tempo() {}

public:
	void setLeftNoteType(int type);//NoteType
	NoteType getLeftNoteType() const;

	void setShowMark(bool show);
	bool getShowMark() const;

	void setShowBeforeText(bool show);
	bool getShowBeforeText() const;

	void setShowParenthesis(bool show);
	bool getShowParenthesis() const;

	void setTypeTempo(int tempo); //0x2580 = 96.00
	int getTypeTempo() const;
	int getQuarterTempo() const;

	void setLeftText(const QString& str);// string at left of the mark
	QString getLeftText() const;

	void setRightText(const QString& str);
	QString getRightText() const;

	void setSwingEighth(bool swing);
	bool getSwingEighth() const;

	void setRightNoteType(int type);
	int getRightNoteType() const;

private:
	int leftNoteType_;
	bool showMark_;
	bool showText_;
	bool showParenthesis_;
	int typeTempo_;
	QString leftText_;
	QString rightText_;
	bool swingEighth_;
	int rightNoteType_;
};

class Text: public MusicData, public LengthElement {
public:
	Text();
	virtual ~Text() {}

public:
	enum TextType {
		Text_Rehearsal,
		Text_SystemText,
		Text_MeasureText
	};

	void setTextType(TextType type);
	TextType getTextType() const;

	void setHorizontalMargin(int margin);
	int getHorizontalMargin() const;

	void setVerticalMargin(int margin);
	int getVerticalMargin() const;

	void setLineThick(int thick);
	int getLineThick() const;

	void setText(const QString& text);
	QString getText() const;

	void setWidth(int width);
	int getWidth() const;

	void setHeight(int height);
	int getHeight() const;

private:
	TextType textType_;
	int horiMargin_;
	int vertMargin_;
	int lineThick_;
	QString text_;
	int width_;
	int height_;
};

///////////////////////////////////////////////////////////////////////////////

class TimeSignature: public MusicData {
public:
	TimeSignature();
	virtual ~TimeSignature() {}

public:
	void setNumerator(int numerator);
	int getNumerator() const;

	void setDenominator(int denominator);
	int getDenominator() const;

	void setIsSymbol(bool symbol); //4/4:common, 2/2:cut
	bool getIsSymbol() const;

	void setBeatLength(int length); // tick
	int getBeatLength() const;

	void setBarLength(int length); // tick
	int getBarLength() const;

	void addBeat(int startUnit, int lengthUnit, int startTick);
	void endAddBeat();
	int getUnits() const;

	void setReplaceFont(bool replace);
	bool getReplaceFont() const;

	void setShowBeatGroup(bool show);
	bool getShowBeatGroup() const;

	void setGroupNumerator1(int numerator);
	void setGroupNumerator2(int numerator);
	void setGroupNumerator3(int numerator);
	void setGroupDenominator1(int denominator);
	void setGroupDenominator2(int denominator);
	void setGroupDenominator3(int denominator);

	void setBeamGroup1(int count);
	void setBeamGroup2(int count);
	void setBeamGroup3(int count);
	void setBeamGroup4(int count);

	void set16thBeamCount(int count);
	void set32thBeamCount(int count);

private:
	int numerator_;
	int denominator_;
	bool isSymbol_;
	int beatLength_;
	int barLength_;

	struct BeatNode {
		int startUnit_;
		int lengthUnit_;
		int startTick_;

		BeatNode() :
			startUnit_(0),
			lengthUnit_(0),
			startTick_(0) {
		}
	};
	QList<BeatNode> beats_;
	int barLengthUnits_;

	bool replaceFont_;
	bool showBeatGroup_;

	int groupNumerator1_;
	int groupNumerator2_;
	int groupNumerator3_;
	int groupDenominator1_;
	int groupDenominator2_;
	int groupDenominator3_;

	int beamGroup1_;
	int beamGroup2_;
	int beamGroup3_;
	int beamGroup4_;

	int beamCount16th_;
	int beamCount32th_;
};

class Key: public MusicData {
public:
	Key();
	virtual ~Key() {}

public:
	void setKey(int key); //C=0x0, G=0x8, C#=0xE, F=0x1, Db=0x7
	int getKey() const;
	bool getSetKey() const;

	void setPreviousKey(int key);
	int getPreviousKey() const;

	void setSymbolCount(int count);
	int getSymbolCount() const;

private:
	int key_;
	bool set_;
	int previousKey_;
	int symbolCount_;
};

class RepeatSymbol: public MusicData {
public:
	RepeatSymbol();
	virtual ~RepeatSymbol() {}

public:
	void setText(const QString& text);
	QString getText() const;

	void setRepeatType(int repeatType);
	RepeatType getRepeatType() const;

private:
	QString text_;
	RepeatType repeatType_;
};

class NumericEnding: public MusicData, public PairEnds {
public:
	NumericEnding();
	virtual ~NumericEnding();

public:
	OffsetElement* getNumericHandle() const;

	void setHeight(int height);
	int getHeight() const;

	void setText(const QString& text);
	QString getText() const;
	QList<int> getNumbers() const;
	int getJumpCount() const;

private:
	int height_;
	QString text_;
	OffsetElement* numericHandle_;
};

class BarNumber: public MusicData {
public:
	BarNumber();
	virtual ~BarNumber() {}

public:
	void setIndex(int index);
	int getIndex() const;

	void setShowOnParagraphStart(bool show);
	bool getShowOnParagraphStart() const;

	void setAlign(int align);// 0:left, 1:center, 2:right
	int getAlign() const;

	void setShowFlag(int flag); // 0:page, 1:staff, 2:bar, 3:none
	int getShowFlag() const;

	void setShowEveryBarCount(int count);
	int getShowEveryBarCount() const;

	void setPrefix(const QString& str);
	QString getPrefix() const;

private:
	int index_;
	bool showOnParagraphStart_;
	int align_;
	int showFlag_;
	int barRange_;
	QString prefix_;
};

///////////////////////////////////////////////////////////////////////////////
// MIDI
class MidiController: public MidiData {
public:
	MidiController();
	virtual ~MidiController() {}

public:
	void setController(int number);
	int getController() const;

	void setValue(int value);
	int getValue() const;

private:
	int controller_;
	int value_;
};

class MidiProgramChange: public MidiData {
public:
	MidiProgramChange();
	virtual ~MidiProgramChange() {}

public:
	void setPatch(int patch);
	int getPatch() const;

private:
	int patch_;
};

class MidiChannelPressure: public MidiData {
public:
	MidiChannelPressure();
	virtual ~MidiChannelPressure() {}

public:
	void setPressure(int pressure);
	int getPressure() const;

private:
	int pressure_;
};

class MidiPitchWheel: public MidiData {
public:
	MidiPitchWheel();
	virtual ~MidiPitchWheel() {}

public:
	void setValue(int value);
	int getValue() const;

private:
	int value_;
};

///////////////////////////////////////////////////////////////////////////////
class Measure: public LengthElement {
public:
	Measure(int index = 0);
	virtual ~Measure();

private:
	Measure();

public:
	BarNumber* getBarNumber() const;
	TimeSignature* getTime() const;

	void setLeftBarline(int barline/*in BarlineType*/);
	BarlineType getLeftBarline() const;

	void setRightBarline(int barline/*in BarlineType*/);
	BarlineType getRightBarline() const;

	// set when rightBarline == Baline_Backward
	void setBackwardRepeatCount(int repeatCount);
	int getBackwardRepeatCount() const;

	void setTypeTempo(double tempo);
	double getTypeTempo() const;

	void setIsPickup(bool pickup);
	bool getIsPickup() const;

	void setIsMultiMeasureRest(bool rest);
	bool getIsMultiMeasureRest() const;

	void setMultiMeasureRestCount(int count);
	int getMultiMeasureRestCount() const;

private:
	void clear();

	BarNumber* barNumber_;
	TimeSignature* time_;

	BarlineType leftBarline_;
	BarlineType rightBarline_;
	int repeatCount_;
	double typeTempo_; // based on some type
	bool pickup_;
	bool multiMeasureRest_;
	int multiMeasureRestCount_;
};

class MeasureData {
public:
	MeasureData();
	~MeasureData();

public:
	Clef* getClef() const;
	Key* getKey() const;

	void addNoteContainer(NoteContainer* ptr);
	QList<NoteContainer*> getNoteContainers() const;

	// put Tempo, Text, RepeatSymbol to MeasureData at part=0 && staff=0
	void addMusicData(MusicData* ptr);
	// if type==MusicData_None, return all
	QList<MusicData*> getMusicDatas(MusicDataType type);//MusicXml: note|direction|harmony

	// put NumericEnding to MeasureData at part=0 && staff=0
	void addCrossMeasureElement(MusicData* ptr, bool start);
	enum PairType {
		PairType_Start,
		PairType_Stop,
		PairType_All
	};
	QList<MusicData*> getCrossMeasureElements(MusicDataType type, PairType pairType);

	// for midi
	void addMidiData(MidiData* ptr);
	QList<MidiData*> getMidiDatas(MidiType type);

private:
	Key* key_;
	Clef* clef_;
	QList<MusicData*> musicDatas_;
	QList<NoteContainer*> noteContainers_;
	QList<QPair<MusicData*, bool> > crossMeasureElements_;
	QList<MidiData*> midiDatas_;
};

// StreamHandle
class StreamHandle {
public:
	StreamHandle(unsigned char* p, int size);
	virtual ~StreamHandle();

private:
	StreamHandle();

public:
	virtual bool read(char* buff, int size);
	virtual bool write(char* buff, int size);

private:
	int size_;
	int curPos_;
	unsigned char* point_;
};

// Block.h
// base block, or resizable block in ove to store data
class Block {
public:
	Block();
	explicit Block(unsigned int size);
	virtual ~Block() {
	}

public:
	// size > 0, check this in use code
	virtual void resize(unsigned int count);

	const unsigned char* data() const;
	unsigned char* data();
	unsigned int size() const;

	bool operator ==(const Block& block) const;
	bool operator !=(const Block& block) const;

	bool toBoolean() const;
	unsigned int toUnsignedInt() const;
	int toInt() const;
	QByteArray toStrByteArray() const;					// string
	QByteArray fixedSizeBufferToStrByteArray() const;	// string

private:
	void doResize(unsigned int count);

private:
	// char [-128, 127], unsigned char [0, 255]
	QList<unsigned char> data_;
};

class FixedBlock: public Block {
public:
	explicit FixedBlock(unsigned int count);
	virtual ~FixedBlock() {
	}

private:
	FixedBlock();

private:
	// can't resize
	virtual void resize(unsigned int count);
};

///////////////////////////////////////////////////////////////////////////////
// 4 byte block in ove to store size
class SizeBlock: public FixedBlock {
public:
	SizeBlock();
	virtual ~SizeBlock() {
	}

public:
	//	void fromUnsignedInt(unsigned int count) ;

	unsigned int toSize() const;
};

// 4 bytes block in ove to store name
class NameBlock: public FixedBlock {
public:
	NameBlock();
	virtual ~NameBlock() {
	}

public:
	// ingore data more than 4 bytes
	bool isEqual(const QString& name) const;
};

// 2 bytes block in ove to store count
class CountBlock: public FixedBlock {
public:
	CountBlock();
	virtual ~CountBlock() {
	}

public:
	//	void setValue(unsigned short count) ;

	unsigned short toCount() const;
};

// Chunk.h
// content : name
class Chunk {
public:
	Chunk();
	virtual ~Chunk() {
	}

public:
	const static QString TrackName;
	const static QString PageName;
	const static QString LineName;
	const static QString StaffName;
	const static QString MeasureName;
	const static QString ConductName;
	const static QString BdatName;

	NameBlock getName() const;

protected:
	NameBlock nameBlock_;
};

// content : name / size / data
class SizeChunk: public Chunk {
public:
	SizeChunk();
	virtual ~SizeChunk();

public:
	SizeBlock* getSizeBlock() const;
	Block* getDataBlock() const;

	const static unsigned int version3TrackSize;

protected:
	SizeBlock* sizeBlock_;
	Block* dataBlock_;
};

// content : name / count
class GroupChunk: public Chunk {
public:
	GroupChunk();
	virtual ~GroupChunk();

public:
	CountBlock* getCountBlock() const;

protected:
	CountBlock* childCount_;
};

// ChunkParse.h
class BasicParse {
public:
	BasicParse(OveSong* ove);
	virtual ~BasicParse();

private:
	BasicParse();

public:
	void setNotify(IOveNotify* notify);
	virtual bool parse();

protected:
	bool readBuffer(Block& placeHolder, unsigned int size);
	bool jump(int offset);

	void messageOut(const QString& str);

protected:
	OveSong* ove_;
	StreamHandle* handle_;
	IOveNotify* notify_;
};

///////////////////////////////////////////////////////////////////////////////

class OvscParse: public BasicParse {
public:
	OvscParse(OveSong* ove);
	virtual ~OvscParse();

public:
	void setOvsc(SizeChunk* chunk);

	virtual bool parse();

private:
	SizeChunk* chunk_;
};

class TrackParse: public BasicParse {
public:
	TrackParse(OveSong* ove);
	virtual ~TrackParse();

public:
	void setTrack(SizeChunk* chunk);

	virtual bool parse();

private:
	SizeChunk* chunk_;
};

class GroupParse: BasicParse {
public:
	GroupParse(OveSong* ove);
	virtual ~GroupParse();

public:
	void addSizeChunk(SizeChunk* sizeChunk);

	virtual bool parse();

private:
	QList<SizeChunk*> sizeChunks_;
};

class PageGroupParse: public BasicParse {
public:
	PageGroupParse(OveSong* ove);
	virtual ~PageGroupParse();

public:
	void addPage(SizeChunk* chunk);

	virtual bool parse();

private:
	bool parsePage(SizeChunk* chunk, Page* page);

private:
	QList<SizeChunk*> pageChunks_;
};

class StaffCountGetter: public BasicParse {
public:
	StaffCountGetter(OveSong* ove);
	virtual ~StaffCountGetter() {}

public:
	unsigned int getStaffCount(SizeChunk* chunk);
};

class LineGroupParse: public BasicParse {
public:
	LineGroupParse(OveSong* ove);
	virtual ~LineGroupParse();

public:
	void setLineGroup(GroupChunk* chunk);
	void addLine(SizeChunk* chunk);
	void addStaff(SizeChunk* chunk);

	virtual bool parse();

private:
	bool parseLine(SizeChunk* chunk, Line* line);
	bool parseStaff(SizeChunk* chunk, Staff* staff);

private:
	GroupChunk* chunk_;
	QList<SizeChunk*> lineChunks_;
	QList<SizeChunk*> staffChunks_;
};

class BarsParse: public BasicParse {
public:
	BarsParse(OveSong* ove);
	virtual ~BarsParse();

public:
	void addMeasure(SizeChunk* chunk);
	void addConduct(SizeChunk* chunk);
	void addBdat(SizeChunk* chunk);

	virtual bool parse();

private:
	bool parseMeas(Measure* measure, SizeChunk* chunk);
	bool parseCond(Measure* measure, MeasureData* measureData, SizeChunk* chunk);
	bool parseBdat(Measure* measure, MeasureData* measureData, SizeChunk* chunk);

	bool getCondElementType(unsigned int byteData, CondType& type);
	bool getBdatElementType(unsigned int byteData, BdatType& type);

	// COND
	bool parseTimeSignature(Measure* measure, int length);
	bool parseTimeSignatureParameters(Measure* measure, int length);
	bool parseRepeatSymbol(MeasureData* measureData, int length);
	bool parseNumericEndings(MeasureData* measureData, int length);
	bool parseTempo(MeasureData* measureData, int length);
	bool parseBarNumber(Measure* measure, int length);
	bool parseText(MeasureData* measureData, int length);
	bool parseBarlineParameters(Measure* measure, int length);

	// BDAT
	bool parseNoteRest(MeasureData* measureData, int length, BdatType type);
	bool parseBeam(MeasureData* measureData, int length);
	bool parseTie(MeasureData* measureData, int length);
	bool parseTuplet(MeasureData* measureData, int length);
	bool parseHarmony(MeasureData* measureData, int length);
	bool parseClef(MeasureData* measureData, int length);
	bool parseLyric(MeasureData* measureData, int length);
	bool parseSlur(MeasureData* measureData, int length);
	bool parseGlissando(MeasureData* measureData, int length);
	bool parseDecorators(MeasureData* measureData, int length);
	bool parseDynamics(MeasureData* measureData, int length);
	bool parseWedge(MeasureData* measureData, int length);
	bool parseKey(MeasureData* measureData, int length);
	bool parsePedal(MeasureData* measureData, int length);
	bool parseKuohao(MeasureData* measureData, int length);
	bool parseExpressions(MeasureData* measureData, int length);
	bool parseHarpPedal(MeasureData* measureData, int length);
	bool parseMultiMeasureRest(MeasureData* measureData, int length);
	bool parseHarmonyGuitarFrame(MeasureData* measureData, int length);
	bool parseOctaveShift(MeasureData* measureData, int length);
	bool parseMidiController(MeasureData* measureData, int length);
	bool parseMidiProgramChange(MeasureData* measureData, int length);
	bool parseMidiChannelPressure(MeasureData* measureData, int length);
	bool parseMidiPitchWheel(MeasureData* measureData, int length);

	bool parseSizeBlock(int length);
	bool parseMidiCommon(MidiData* ptr);
	bool parseCommonBlock(MusicData* ptr);
	bool parseOffsetCommonBlock(MusicData* ptr);
	bool parsePairLinesBlock(PairEnds* ptr); //size==2
	bool parseOffsetElement(OffsetElement* ptr);//size==2

private:
	QList<SizeChunk*> measureChunks_;
	QList<SizeChunk*> conductChunks_;
	QList<SizeChunk*> bdatChunks_;
};

class LyricChunkParse: public BasicParse {
public:
	LyricChunkParse(OveSong* ove);
	virtual ~LyricChunkParse() {}

public:
	void setLyricChunk(SizeChunk* chunk);

	virtual bool parse();

private:
	struct LyricInfo {
		int track_;
		int measure_;
		int verse_;
		int voice_;
		int wordCount_;
		int lyricSize_;
		QString name_;
		QString lyric_;
		int font_;
		int fontSize_;
		int fontStyle_;

		LyricInfo() :
			track_(0), measure_(0), verse_(0), voice_(0), wordCount_(0),
					lyricSize_(0), name_(QString()), lyric_(QString()),
					font_(0), fontSize_(12), fontStyle_(0) {}
	};

	void processLyricInfo(const LyricInfo& info);

private:
	SizeChunk* chunk_;
};

class TitleChunkParse: public BasicParse {
public:
	TitleChunkParse(OveSong* ove);
	virtual ~TitleChunkParse() {}

public:
	void setTitleChunk(SizeChunk* chunk);

	virtual bool parse();

private:
	void addToOve(const QString& str, unsigned int titleType);

private:
	unsigned int titleType_;
	unsigned int annotateType_;
	unsigned int writerType_;
	unsigned int copyrightType_;
	unsigned int headerType_;
	unsigned int footerType_;

	SizeChunk* chunk_;
};

// OveOrganizer.h
class OveOrganizer {
public:
	OveOrganizer(OveSong* ove) ;
	virtual ~OveOrganizer(){}

public:
	void organize() ;

private:
	void organizeAttributes() ;
	void organizeTracks() ;
	void organizeMeasures() ;
	void organizeMeasure(int part, int track, Measure* measure, MeasureData* measureData) ;

	void organizeContainers(int part, int track, Measure* measure, MeasureData* measureData) ;
	void organizeMusicDatas(int part, int track, Measure* measure, MeasureData* measureData) ;
	void organizeCrossMeasureElements(int part, int track, Measure* measure, MeasureData* measureData) ;

	void organizePairElement(MusicData* data, int part, int track, Measure* measure, MeasureData* measureData) ;
	void organizeOctaveShift(OctaveShift* octave, Measure* measure, MeasureData* measureData) ;
	void organizeWedge(Wedge* wedge, int part, int track, Measure* measure, MeasureData* measureData) ;

private:
	OveSong* ove_ ;
};

// OveSerialize.h
class StreamHandle;
class Block;
class NameBlock;
class Chunk;
class SizeChunk;
class GroupChunk;

class OveSerialize: public IOVEStreamLoader {
public:
	OveSerialize();
	virtual ~OveSerialize();

public:
	virtual void setOve(OveSong* ove);
	virtual void setFileStream(unsigned char* buffer, unsigned int size);
	virtual void setNotify(IOveNotify* notify);
	virtual bool load(void);

	virtual void release();

private:
	bool readNameBlock(NameBlock& nameBlock);
	bool readChunkName(Chunk* chunk, const QString& name);
	bool readSizeChunk(SizeChunk* sizeChunk); // contains a SizeChunk and data buffer
	bool readDataChunk(Block* block, unsigned int size);
	bool readGroupChunk(GroupChunk* groupChunk);

	bool readHeader();
	bool readHeadData(SizeChunk* ovscChunk);
	bool readTracksData();
	bool readPagesData();
	bool readLinesData();
	bool readBarsData();
	bool readOveEnd();

	void messageOutError();
	void messageOut(const QString& str);

private:
	OveSong* ove_;
	StreamHandle* streamHandle_;
	IOveNotify* notify_;
};

/*template <class T>
inline void deleteVector(QList<T*>& vec) {
	for (int i=0; i<vec.size(); ++i)
		delete vec[i];
	}
	//vec.clear();
}*/

///////////////////////////////////////////////////////////////////////////////
TickElement::TickElement() {
	tick_ = 0;
}

void TickElement::setTick(int tick) {
	tick_ = tick;
}

int TickElement::getTick(void) const {
	return tick_;
}

///////////////////////////////////////////////////////////////////////////////
MeasurePos::MeasurePos() {
	measure_ = 0;
	offset_ = 0;
}

void MeasurePos::setMeasure(int measure) {
	measure_ = measure;
}

int MeasurePos::getMeasure() const {
	return measure_;
}

void MeasurePos::setOffset(int offset) {
	offset_ = offset;
}

int MeasurePos::getOffset() const {
	return offset_;
}

MeasurePos MeasurePos::shiftMeasure(int measure) const {
	MeasurePos mp;
	mp.setMeasure(getMeasure() + measure);
	mp.setOffset(getOffset());

	return mp;
}

MeasurePos MeasurePos::shiftOffset(int offset) const {
	MeasurePos mp;
	mp.setMeasure(getMeasure());
	mp.setOffset(getOffset() + offset);

	return mp;
}

bool MeasurePos::operator ==(const MeasurePos& mp) const {
	return getMeasure() == mp.getMeasure() && getOffset() == mp.getOffset();
}

bool MeasurePos::operator !=(const MeasurePos& mp) const {
	return !(*this == mp);
}

bool MeasurePos::operator <(const MeasurePos& mp) const {
	if (getMeasure() != mp.getMeasure()) {
		return getMeasure() < mp.getMeasure();
	}

	return getOffset() < mp.getOffset();
}

bool MeasurePos::operator <=(const MeasurePos& mp) const {
	if (getMeasure() != mp.getMeasure()) {
		return getMeasure() <= mp.getMeasure();
	}

	return getOffset() <= mp.getOffset();
}

bool MeasurePos::operator >(const MeasurePos& mp) const {
	return !(*this <= mp);
}

bool MeasurePos::operator >=(const MeasurePos& mp) const {
	return !(*this < mp);
}

///////////////////////////////////////////////////////////////////////////////
PairElement::PairElement() {
	start_ = new MeasurePos();
	stop_ = new MeasurePos();
}

PairElement::~PairElement(){
	delete start_;
	delete stop_;
}

MeasurePos* PairElement::start() const {
	return start_;
}

MeasurePos* PairElement::stop() const {
	return stop_;
}

///////////////////////////////////////////////////////////////////////////////
PairEnds::PairEnds() {
	leftLine_ = new LineElement();
	rightLine_ = new LineElement();
	leftShoulder_ = new OffsetElement();
	rightShoulder_ = new OffsetElement();
}

PairEnds::~PairEnds(){
	delete leftLine_;
	delete rightLine_;
	delete leftShoulder_;
	delete rightShoulder_;
}

LineElement* PairEnds::getLeftLine() const {
	return leftLine_;
}

LineElement* PairEnds::getRightLine() const {
	return rightLine_;
}

OffsetElement* PairEnds::getLeftShoulder() const {
	return leftShoulder_;
}

OffsetElement* PairEnds::getRightShoulder() const {
	return rightShoulder_;
}

///////////////////////////////////////////////////////////////////////////////
LineElement::LineElement() {
	line_ = 0;
}

void LineElement::setLine(int line) {
	line_ = line;
}

int LineElement::getLine(void) const {
	return line_;
}

///////////////////////////////////////////////////////////////////////////////
OffsetElement::OffsetElement() {
	xOffset_ = 0;
	yOffset_ = 0;
}

void OffsetElement::setXOffset(int offset) {
	xOffset_ = offset;
}

int OffsetElement::getXOffset() const {
	return xOffset_;
}

void OffsetElement::setYOffset(int offset) {
	yOffset_ = offset;
}

int OffsetElement::getYOffset() const {
	return yOffset_;
}

///////////////////////////////////////////////////////////////////////////////
LengthElement::LengthElement() {
	length_ = 0;
}

void LengthElement::setLength(int length) {
	length_ = length;
}

int LengthElement::getLength() const {
	return length_;
}

///////////////////////////////////////////////////////////////////////////////
MusicData::MusicData() {
	musicDataType_ = MusicData_None;
	show_ = true;
	color_ = 0;
	voice_ = 0;
}

MusicDataType MusicData::getMusicDataType() const {
	return musicDataType_;
}

MusicData::XmlDataType MusicData::getXmlDataType(MusicDataType type) {
	XmlDataType xmlType = None;

	switch (type) {
	case MusicData_Measure_Repeat: {
		xmlType = Attributes;
		break;
	}
	case MusicData_Beam: {
		xmlType = NoteBeam;
		break;
	}
	case MusicData_Slur:
	case MusicData_Glissando:
	case MusicData_Tuplet:
	case MusicData_Tie: {
		xmlType = Notations;
		break;
	}
	case MusicData_Text:
	case MusicData_Repeat:
	case MusicData_Wedge:
	case MusicData_Dynamics:
	case MusicData_Pedal:
	case MusicData_OctaveShift_EndPoint: {
		xmlType = Direction;
		break;
	}
	default: {
		xmlType = None;
		break;
	}
	}

	return xmlType;
}

/*bool MusicData::get_is_pair_element(MusicDataType type)
 {
 bool pair = false;

 switch ( type )
 {
 case MusicData_Numeric_Ending :
 case MusicData_Measure_Repeat :
 case MusicData_Wedge :
 case MusicData_OctaveShift :
 //case MusicData_OctaveShift_EndPoint :
 case MusicData_Pedal :
 case MusicData_Beam :
 case MusicData_Glissando :
 case MusicData_Slur :
 case MusicData_Tie :
 case MusicData_Tuplet :
 {
 pair = true;
 break;
 }
 default:
 break;
 }

 return pair;
 }*/

void MusicData::setShow(bool show) {
	show_ = show;
}

bool MusicData::getShow() const {
	return show_;
}

void MusicData::setColor(unsigned int color) {
	color_ = color;
}

unsigned int MusicData::getColor() const {
	return color_;
}

void MusicData::setVoice(unsigned int voice) {
	voice_ = voice;
}

unsigned int MusicData::getVoice() const {
	return voice_;
}

void MusicData::copyCommonBlock(const MusicData& source) {
	setTick(source.getTick());
	start()->setOffset(source.start()->getOffset());
	setColor(source.getColor());
}

///////////////////////////////////////////////////////////////////////////////
MidiData::MidiData() {
	midiType_ = Midi_None;
}

MidiType MidiData::getMidiType() const {
	return midiType_;
}

///////////////////////////////////////////////////////////////////////////////
OveSong::OveSong() :
	codec_(0) {
	clear();
}

OveSong::~OveSong() {
	clear();
}

void OveSong::setIsVersion4(bool version4){
	version4_ = version4;
}

bool OveSong::getIsVersion4() const {
	return version4_;
}

void OveSong::setQuarter(int tick) {
	quarter_ = tick;
}

int OveSong::getQuarter(void) const {
	return quarter_;
}

void OveSong::setShowPageMargin(bool show){
	showPageMargin_ = show;
}

bool OveSong::getShowPageMargin() const {
	return showPageMargin_;
}

void OveSong::setShowTransposeTrack(bool show) {
	showTransposeTrack = show;
}

bool OveSong::getShowTransposeTrack() const {
	return showTransposeTrack;
}

void OveSong::setShowLineBreak(bool show) {
	showLineBreak_ = show;
}

bool OveSong::getShowLineBreak() const {
	return showLineBreak_;
}

void OveSong::setShowRuler(bool show) {
	showRuler_ = show;
}

bool OveSong::getShowRuler() const {
	return showRuler_;
}

void OveSong::setShowColor(bool show) {
	showColor_ = show;
}

bool OveSong::getShowColor() const {
	return showColor_;
}

void OveSong::setPlayRepeat(bool play) {
	playRepeat_ = play;
}

bool OveSong::getPlayRepeat() const {
	return playRepeat_;
}

void OveSong::setPlayStyle(PlayStyle style) {
	playStyle_ = style;
}

OveSong::PlayStyle OveSong::getPlayStyle() const {
	return playStyle_;
}

void OveSong::addTitle(const QString& str) {
	titles_.push_back(str);
}

QList<QString> OveSong::getTitles(void) const {
	return titles_;
}

void OveSong::addAnnotate(const QString& str) {
	annotates_.push_back(str);
}

QList<QString> OveSong::getAnnotates(void) const {
	return annotates_;
}

void OveSong::addWriter(const QString& str) {
	writers_.push_back(str);
}

QList<QString> OveSong::getWriters(void) const {
	return writers_;
}

void OveSong::addCopyright(const QString& str) {
	copyrights_.push_back(str);
}

QList<QString> OveSong::getCopyrights(void) const {
	return copyrights_;
}

void OveSong::addHeader(const QString& str) {
	headers_.push_back(str);
}

QList<QString> OveSong::getHeaders(void) const {
	return headers_;
}

void OveSong::addFooter(const QString& str) {
	footers_.push_back(str);
}

QList<QString> OveSong::getFooters(void) const {
	return footers_;
}

void OveSong::addTrack(Track* ptr) {
	tracks_.push_back(ptr);
}

int OveSong::getTrackCount(void) const {
	return tracks_.size();
}

QList<Track*> OveSong::getTracks() const {
	return tracks_;
}

void OveSong::setTrackBarCount(int count) {
	trackBarCount_ = count;
}

int OveSong::getTrackBarCount() const {
	return trackBarCount_;
}

Track* OveSong::getTrack(int part, int staff) const {
	int trackId = partStaffToTrack(part, staff);

	if( trackId >=0 && trackId < (int)tracks_.size() ) {
		return tracks_[trackId];
	}

	return 0;
}

bool OveSong::addPage(Page* page) {
	pages_.push_back(page);
	return true;
}

int OveSong::getPageCount() const {
	return pages_.size();
}

Page* OveSong::getPage(int idx) {
	if( idx>=0 && idx<(int)pages_.size() ) {
		return pages_[idx];
	}

	return 0;
}

void OveSong::addLine(Line* ptr) {
	lines_.push_back(ptr);
}

int OveSong::getLineCount() const {
	return lines_.size();
}

Line* OveSong::getLine(int idx) const {
	if( idx >=0 && idx<(int)lines_.size() ) {
		return lines_[idx];
	}

	return 0;
}

void OveSong::addMeasure(Measure* ptr) {
	measures_.push_back(ptr);
}

int OveSong::getMeasureCount(void) const {
	return measures_.size();
}

Measure* OveSong::getMeasure(int bar) const {
	if( bar >= 0 && bar < (int)measures_.size() ) {
		return measures_[bar];
	}

	return 0;
}

void OveSong::addMeasureData(MeasureData* ptr) {
	measureDatas_.push_back(ptr);
}

int OveSong::getMeasureDataCount(void) const {
	return measureDatas_.size();
}

MeasureData* OveSong::getMeasureData(int part, int staff/*=0*/, int bar) const {
	int trackId = partStaffToTrack(part, staff);
	int trackBarCount = getTrackBarCount();

	if( bar >= 0 && bar < trackBarCount ) {
		int measureId = trackBarCount * trackId + bar;

		if( measureId >=0 && measureId < (int)measureDatas_.size() ) {
			return measureDatas_[measureId];
		}
	}

	return 0;
}

MeasureData* OveSong::getMeasureData(int track, int bar) const {
	int id = trackBarCount_*track + bar;

	if( id >=0 && id < (int)measureDatas_.size() ) {
		return measureDatas_[id];
	}

	return 0;
}

void OveSong::setPartStaffCounts(const QList<int>& partStaffCounts) {
	//partStaffCounts_.assign(partStaffCounts.begin(), partStaffCounts.end());
	for(int i=0; i<partStaffCounts.size(); ++i) {
		partStaffCounts_.push_back(partStaffCounts[i]);
	}
}

int OveSong::getPartCount() const {
	return partStaffCounts_.size();
}

int OveSong::getStaffCount(int part) const {
	if( part>=0 && part<(int)partStaffCounts_.size() ) {
		return partStaffCounts_[part];
	}

	return 0;
}

int OveSong::getPartBarCount() const {
	return measureDatas_.size() / tracks_.size();
}

QPair<int, int> OveSong::trackToPartStaff(int track) const {
	QPair<int, int> partStaff;
	unsigned int i;
	int staffCount = 0;

	for( i=0; i<partStaffCounts_.size(); ++i ) {
		if( staffCount + partStaffCounts_[i] > track ) {
			return qMakePair((int)i, track-staffCount);
		}

		staffCount += partStaffCounts_[i];
	}

	return qMakePair((int)partStaffCounts_.size(), 0);
}

int OveSong::partStaffToTrack(int part, int staff) const {
	unsigned int i;
	unsigned int staffCount = 0;

	for( i=0; i<partStaffCounts_.size(); ++i ) {
		if( part == (int)i && staff>=0 && staff<(int)partStaffCounts_[i] ) {
			int trackId = staffCount + staff;

			if( trackId >=0 && trackId < (int)tracks_.size() ) {
				return trackId;
			}
		}

		staffCount += partStaffCounts_[i];
	}

	return tracks_.size();
}

void OveSong::setTextCodecName(const QString& codecName) {
	codec_ = QTextCodec::codecForName(codecName.toLatin1());
}

QString OveSong::getCodecString(const QByteArray& text) {
    QString s;
    if (codec_ == NULL)
        s = QString(text);
    else
        s = codec_->toUnicode(text);

    return s;
}

void OveSong::clear(void) {
	version4_ = true;
	quarter_ = 480;
	showPageMargin_ = false;
	showTransposeTrack = false;
	showLineBreak_ = false;
	showRuler_ = false;
	showColor_ = true;
	playRepeat_ = true;
	playStyle_ = Record;

	annotates_.clear();
	copyrights_.clear();
	footers_.clear();
	headers_.clear();
	titles_.clear();
	writers_.clear();

//	deleteVector(tracks_);
	for(unsigned int i=0; i<tracks_.size(); ++i){
		delete tracks_[i];
	}
	for(unsigned int i=0; i<pages_.size(); ++i){
		delete pages_[i];
	}
	for(unsigned int i=0; i<lines_.size(); ++i){
		delete lines_[i];
	}
	for(unsigned int i=0; i<measures_.size(); ++i){
		delete measures_[i];
	}
	for(unsigned int i=0; i<measureDatas_.size(); ++i){
		delete measureDatas_[i];
	}
	tracks_.clear();
	pages_.clear();
	lines_.clear();
	measures_.clear();
	measureDatas_.clear();
	trackBarCount_ = 0;
	partStaffCounts_.clear();
}

///////////////////////////////////////////////////////////////////////////////
Voice::Voice() {
	channel_ = 0;
	volume_ = -1;
	pitchShift_ = 0;
	pan_ = 0;
	patch_ = 0;
	stemType_ = 0;
}

void Voice::setChannel(int channel) {
	channel_ = channel;
}

int Voice::getChannel() const {
	return channel_;
}

void Voice::setVolume(int volume) {
	volume_ = volume;
}

int Voice::getVolume() const {
	return volume_;
}

void Voice::setPitchShift(int pitchShift) {
	pitchShift_ = pitchShift;
}

int Voice::getPitchShift() const {
	return pitchShift_;
}

void Voice::setPan(int pan) {
	pan_ = pan;
}

int Voice::getPan() const {
	return pan_;
}

void Voice::setPatch(int patch) {
	patch_ = patch;
}

int Voice::getPatch() const {
	return patch_;
}

void Voice::setStemType(int stemType) {
	stemType_ = stemType;
}

int Voice::getStemType() const {
	return stemType_;
}

int Voice::getDefaultPatch() {
	return -1;
}

int Voice::getDefaultVolume() {
	return -1;
}

///////////////////////////////////////////////////////////////////////////////
Track::Track() {
	clear();
}

Track::~Track() {
	clear();
}

void Track::setName(const QString& str) {
	name_ = str;
}

QString Track::getName(void) const {
	return name_;
}

void Track::setBriefName(const QString& str) {
	briefName_ = str;
}

QString Track::getBriefName(void) const {
	return briefName_;
}

void Track::setPatch(unsigned int patch) {
	patch_ = patch;
}

unsigned int Track::getPatch() const {
	return patch_;
}

void Track::setChannel(int channel) {
	channel_ = channel;
}

int Track::getChannel() const {
	return channel_;
}

void Track::setShowName(bool show) {
	showName_ = show;
}

bool Track::getShowName() const {
	return showName_;
}

void Track::setShowBriefName(bool show) {
	showBriefName_ = show;
}

bool Track::getShowBriefName() const {
	return showBriefName_;
}

void Track::setMute(bool mute) {
	mute_ = mute;
}

bool Track::getMute() const {
	return mute_;
}

void Track::setSolo(bool solo) {
	solo_ = solo;
}

bool Track::getSolo() const {
	return solo_;
}

void Track::setShowKeyEachLine(bool show) {
	showKeyEachLine_ = show;
}

bool Track::getShowKeyEachLine() const {
	return showKeyEachLine_;
}

void Track::setVoiceCount(int voices) {
	voiceCount_ = voices;
}

int Track::getVoiceCount() const {
	return voiceCount_;
}

void Track::addVoice(Voice* voice) {
	voices_.push_back(voice);
}

QList<Voice*> Track::getVoices() const {
	return voices_;
}

void Track::setShowTranspose(bool show) {
	showTranspose_ = show;
}

bool Track::getShowTranspose() const {
	return showTranspose_;
}

void Track::setTranspose(int transpose) {
	transpose_ = transpose;
}

int Track::getTranspose() const {
	return transpose_;
}

void Track::setNoteShift(int shift) {
	noteShift_ = shift;
}

int Track::getNoteShift() const {
	return noteShift_;
}

void Track::setStartClef(int clef/*in Clef*/) {
	startClef_ = clef;
}

ClefType Track::getStartClef() const {
	return (ClefType)startClef_;
}

void Track::setTransposeClef(int clef) {
	transposeClef_ = clef;
}

int Track::getTansposeClef() const {
	return transposeClef_;
}

void Track::setStartKey(int key) {
	startKey_ = key;
}

int Track::getStartKey() const {
	return startKey_;
}

void Track::setDisplayPercent(unsigned int percent/*25~100?*/) {
	displayPercent_ = percent;
}

unsigned int Track::getDisplayPercent() const {
	return displayPercent_;
}

void Track::setShowLegerLine(bool show) {
	showLegerLine_ = show;
}

bool Track::getShowLegerLine() const {
	return showLegerLine_;
}

void Track::setShowClef(bool show) {
	showClef_ = show;
}

bool Track::getShowClef() const {
	return showClef_;
}

void Track::setShowTimeSignature(bool show) {
	showTimeSignature_ = show;
}

bool Track::getShowTimeSignature() const {
	return showTimeSignature_;
}

void Track::setShowKeySignature(bool show) {
	showKeySignature_ = show;
}

bool Track::getShowKeySignature() const {
	return showKeySignature_;
}

void Track::setShowBarline(bool show) {
	showBarline_ = show;
}

bool Track::getShowBarline() const {
	return showBarline_;
}

void Track::setFillWithRest(bool fill) {
	fillWithRest_ = fill;
}

bool Track::getFillWithRest() const {
	return fillWithRest_;
}

void Track::setFlatTail(bool flat) {
	flatTail_ = flat;
}

bool Track::getFlatTail() const {
	return flatTail_;
}

void Track::setShowClefEachLine(bool show) {
	showClefEachLine_ = show;
}

bool Track::getShowClefEachLine() const {
	return showClefEachLine_;
}

void Track::addDrum(const DrumNode& node) {
	/*DrumNode node;
	node.line_ = line;
	node.headType_ = headType;
	node.pitch_ = pitch;
	node.voice_ = voice;*/
	drumKit_.push_back(node);
}

QList<Track::DrumNode> Track::getDrumKit() const {
	return drumKit_;
}

void Track::setPart(int part) {
	part_ = part;
}

int Track::getPart() const {
	return part_;
}

void Track::clear(void) {
	number_ = 0;

	name_ = QString();

	patch_ = 0;
	channel_ = 0;
	transpose_ = 0;
	showTranspose_ = false;
	noteShift_ = 0;
	startClef_ = Clef_Treble;
	transposeClef_ = Clef_Treble;
	displayPercent_ = 100;
	startKey_ = 0;
	voiceCount_ = 8;

	showName_ = true;
	showBriefName_ = false;
	showKeyEachLine_ = false;
	showLegerLine_ = true;
	showClef_ = true;
	showTimeSignature_ = true;
	showKeySignature_ = true;
	showBarline_ = true;
	showClefEachLine_ = false;

	fillWithRest_ = true;
	flatTail_ = false;

	mute_ = false;
	solo_ = false;

	drumKit_.clear();

	part_ = 0;

	for(unsigned int i=0; i<voices_.size(); ++i){
		delete voices_[i];
	}
	voices_.clear();
}

///////////////////////////////////////////////////////////////////////////////
Page::Page() {
	beginLine_ = 0;
	lineCount_ = 0;

	lineInterval_ = 9;
	staffInterval_ = 7;
	staffInlineInterval_ = 6;

	lineBarCount_ = 4;
	pageLineCount_ = 5;

	leftMargin_ = 0xA8;
	topMargin_ = 0xA8;
	rightMargin_ = 0xA8;
	bottomMargin_ = 0xA8;

	pageWidth_ = 0x0B40;
	pageHeight_ = 0x0E90;
}

void Page::setBeginLine(int line) {
	beginLine_ = line;
}

int Page::getBeginLine() const {
	return beginLine_;
}

void Page::setLineCount(int count) {
	lineCount_ = count;
}

int Page::getLineCount() const {
	return lineCount_;
}

void Page::setLineInterval(int interval) {
	lineInterval_ = interval;
}

int Page::getLineInterval() const {
	return lineInterval_;
}

void Page::setStaffInterval(int interval) {
	staffInterval_ = interval;
}

int Page::getStaffInterval() const {
	return staffInterval_;
}

void Page::setStaffInlineInterval(int interval) {
	staffInlineInterval_ = interval;
}

int Page::getStaffInlineInterval() const {
	return staffInlineInterval_;
}

void Page::setLineBarCount(int count) {
	lineBarCount_ = count;
}

int Page::getLineBarCount() const {
	return lineBarCount_;
}

void Page::setPageLineCount(int count) {
	pageLineCount_ = count;
}

int Page::getPageLineCount() const {
	return pageLineCount_;
}

void Page::setLeftMargin(int margin) {
	leftMargin_ = margin;
}

int Page::getLeftMargin() const {
	return leftMargin_;
}

void Page::setTopMargin(int margin) {
	topMargin_ = margin;
}

int Page::getTopMargin() const {
	return topMargin_;
}

void Page::setRightMargin(int margin) {
	rightMargin_ = margin;
}

int Page::getRightMargin() const {
	return rightMargin_;
}

void Page::setBottomMargin(int margin) {
	bottomMargin_ = margin;
}

int Page::getBottomMargin() const {
	return bottomMargin_;
}

void Page::setPageWidth(int width) {
	pageWidth_ = width;
}

int Page::getPageWidth() const {
	return pageWidth_;
}

void Page::setPageHeight(int height) {
	pageHeight_ = height;
}

int Page::getPageHeight() const {
	return pageHeight_;
}

///////////////////////////////////////////////////////////////////////////////
Line::Line() {
	beginBar_ = 0;
	barCount_ = 0;
	yOffset_ = 0;
	leftXOffset_ = 0;
	rightXOffset_ = 0;
}

Line::~Line() {
	for(unsigned int i=0; i<staffs_.size(); ++i){
		delete staffs_[i];
	}
	staffs_.clear();
}

void Line::addStaff(Staff* staff) {
	staffs_.push_back(staff);
}

int Line::getStaffCount() const {
	return staffs_.size();
}

Staff* Line::getStaff(int idx) const {
	if (idx >= 0 && idx < (int) staffs_.size()) {
		return staffs_[idx];
	}

	return 0;
}

void Line::setBeginBar(unsigned int bar) {
	beginBar_ = bar;
}

unsigned int Line::getBeginBar() const {
	return beginBar_;
}

void Line::setBarCount(unsigned int count) {
	barCount_ = count;
}

unsigned int Line::getBarCount() const {
	return barCount_;
}

void Line::setYOffset(int offset) {
	yOffset_ = offset;
}

int Line::getYOffset() const {
	return yOffset_;
}

void Line::setLeftXOffset(int offset) {
	leftXOffset_ = offset;
}

int Line::getLeftXOffset() const {
	return leftXOffset_;
}

void Line::setRightXOffset(int offset) {
	rightXOffset_ = offset;
}

int Line::getRightXOffset() const {
	return rightXOffset_;
}

///////////////////////////////////////////////////////////////////////////////
Staff::Staff() {
	clef_ = Clef_Treble;
	key_ = 0;
	visible_ = true;
	groupType_ = Group_None;
	groupStaffCount_ = 0;
}

void Staff::setClefType(int clef) {
	clef_ = (ClefType) clef;
}

ClefType Staff::getClefType() const {
	return clef_;
}

void Staff::setKeyType(int key) {
	key_ = key;
}

int Staff::getKeyType() const {
	return key_;
}

void Staff::setVisible(bool visible) {
	visible_ = visible;
}

bool Staff::setVisible() const {
	return visible_;
}

void Staff::setGroupType(GroupType type){
	groupType_ = type;
}

GroupType Staff::getGroupType() const {
	return groupType_;
}

void Staff::setGroupStaffCount(int count) {
	groupStaffCount_ = count;
}

int Staff::getGroupStaffCount() const {
	return groupStaffCount_;
}

///////////////////////////////////////////////////////////////////////////////
Note::Note() {
	rest_ = false;
	note_ = 60;
	accidental_ = Accidental_Normal;
	showAccidental_ = false;
	offVelocity_ = 0x40;
	onVelocity_ = 0x50;
	headType_ = NoteHead_Standard;
	tiePos_ = Tie_None;
	offsetStaff_ = 0;
	show_ = true;
	offsetTick_ = 0;
}

void Note::setIsRest(bool rest) {
	rest_ = rest;
}

bool Note::getIsRest() const {
	return rest_;
}

void Note::setNote(unsigned int note) {
	note_ = note;
}

unsigned int Note::getNote() const {
	return note_;
}

void Note::setAccidental(int type) {
	accidental_ = (AccidentalType) type;
}

AccidentalType Note::getAccidental() const {
	return accidental_;
}

void Note::setShowAccidental(bool show) {
	showAccidental_ = show;
}

bool Note::getShowAccidental() const {
	return showAccidental_;
}

void Note::setOnVelocity(unsigned int velocity) {
	onVelocity_ = velocity;
}

unsigned int Note::getOnVelocity() const {
	return onVelocity_;
}

void Note::setOffVelocity(unsigned int velocity) {
	offVelocity_ = velocity;
}

unsigned int Note::getOffVelocity() const {
	return offVelocity_;
}

void Note::setHeadType(int type) {
	headType_ = (NoteHeadType) type;
}

NoteHeadType Note::getHeadType() const {
	return headType_;
}

void Note::setTiePos(int tiePos) {
	tiePos_ = (TiePos) tiePos;
}

TiePos Note::getTiePos() const {
	return tiePos_;
}

void Note::setOffsetStaff(int offset) {
	offsetStaff_ = offset;
}

int Note::getOffsetStaff() const {
	return offsetStaff_;
}

void Note::setShow(bool show) {
	show_ = show;
}

bool Note::getShow() const {
	return show_;
}

void Note::setOffsetTick(int offset) {
	offsetTick_ = offset;
}

int Note::getOffsetTick() const {
	return offsetTick_;
}

///////////////////////////////////////////////////////////////////////////////
Articulation::Articulation() {
	type_ = Articulation_Marcato;
	above_ = true;

	changeSoundEffect_ = false;
	changeLength_ = false;
	changeVelocity_ = false;
	changeExtraLength_ = false;

	soundEffect_ = qMakePair(0, 0);
	lengthPercentage_ = 100;
	velocityType_ = Velocity_Offset;
	velocityValue_ = 0;
	extraLength_ = 0;

	trillNoteLength_ = 60;
	trillRate_ = Note_Sixteen;
	accelerateType_ = Accelerate_None;
	auxiliaryFirst_ = false;
	trillInterval_ = TrillInterval_Chromatic;
}

void Articulation::setArtType(int type) {
	type_ = (ArticulationType) type;
}

ArticulationType Articulation::getArtType() const {
	return type_;
}

void Articulation::setPlacementAbove(bool above) {
	above_ = above;
}

bool Articulation::getPlacementAbove() const {
	return above_;
}

bool Articulation::getChangeSoundEffect() const {
	return changeSoundEffect_;
}

void Articulation::setSoundEffect(int soundFrom, int soundTo) {
	soundEffect_ = qMakePair(soundFrom, soundTo);
	changeSoundEffect_ = true;
}

QPair<int, int> Articulation::getSoundEffect() const {
	return soundEffect_;
}

bool Articulation::getChangeLength() const {
	return changeLength_;
}

void Articulation::setLengthPercentage(int percentage) {
	lengthPercentage_ = percentage;
	changeLength_ = true;
}

int Articulation::getLengthPercentage() const {
	return lengthPercentage_;
}

bool Articulation::getChangeVelocity() const {
	return changeVelocity_;
}

void Articulation::setVelocityType(VelocityType type) {
	velocityType_ = type;
	changeVelocity_ = true;
}

Articulation::VelocityType Articulation::getVelocityType() const {
	return velocityType_;
}

void Articulation::setVelocityValue(int value) {
	velocityValue_ = value;
}

int Articulation::getVelocityValue() const {
	return velocityValue_;
}

bool Articulation::getChangeExtraLength() const {
	return changeExtraLength_;
}

void Articulation::setExtraLength(int length) {
	extraLength_ = length;
	changeExtraLength_ = true;
}

int Articulation::getExtraLength() const {
	return extraLength_;
}

void Articulation::setTrillNoteLength(int length) {
	trillNoteLength_ = length;
}

int Articulation::getTrillNoteLength() const {
	return trillNoteLength_;
}

void Articulation::setTrillRate(NoteType rate) {
	trillRate_ = rate;
}

NoteType Articulation::getTrillRate() const {
	return trillRate_;
}

void Articulation::setAccelerateType(int type) {
	accelerateType_ = (AccelerateType) type;
}

Articulation::AccelerateType Articulation::getAccelerateType() const {
	return accelerateType_;
}

void Articulation::setAuxiliaryFirst(bool first) {
	auxiliaryFirst_ = first;
}

bool Articulation::getAuxiliaryFirst() const {
	return auxiliaryFirst_;
}

void Articulation::setTrillInterval(int interval) {
	trillInterval_ = (TrillInterval) interval;
}

Articulation::TrillInterval Articulation::getTrillInterval() const {
	return trillInterval_;
}

bool Articulation::willAffectNotes() const {
	bool affect = false;

	switch (getArtType()) {
	case Articulation_Major_Trill:
	case Articulation_Minor_Trill:
	case Articulation_Trill_Section:
	case Articulation_Inverted_Short_Mordent:
	case Articulation_Inverted_Long_Mordent:
	case Articulation_Short_Mordent:
	case Articulation_Turn:

	case Articulation_Arpeggio:
	case Articulation_Tremolo_Eighth:
	case Articulation_Tremolo_Sixteenth:
	case Articulation_Tremolo_Thirty_Second:
	case Articulation_Tremolo_Sixty_Fourth: {
		affect = true;
		break;
	}
	case Articulation_Finger_1:
	case Articulation_Finger_2:
	case Articulation_Finger_3:
	case Articulation_Finger_4:
	case Articulation_Finger_5:
	case Articulation_Flat_Accidental_For_Trill:
	case Articulation_Sharp_Accidental_For_Trill:
	case Articulation_Natural_Accidental_For_Trill:
	case Articulation_Marcato:
	case Articulation_Marcato_Dot:
	case Articulation_Heavy_Attack:
	case Articulation_SForzando:
	case Articulation_SForzando_Dot:
	case Articulation_Heavier_Attack:
	case Articulation_SForzando_Inverted:
	case Articulation_SForzando_Dot_Inverted:
	case Articulation_Staccatissimo:
	case Articulation_Staccato:
	case Articulation_Tenuto:
	case Articulation_Up_Bow:
	case Articulation_Down_Bow:
	case Articulation_Up_Bow_Inverted:
	case Articulation_Down_Bow_Inverted:
	case Articulation_Natural_Harmonic:
	case Articulation_Artificial_Harmonic:
	case Articulation_Plus_Sign:
	case Articulation_Fermata:
	case Articulation_Fermata_Inverted:
	case Articulation_Pedal_Down:
	case Articulation_Pedal_Up:
	case Articulation_Pause:
	case Articulation_Grand_Pause:
	case Articulation_Toe_Pedal:
	case Articulation_Heel_Pedal:
	case Articulation_Toe_To_Heel_Pedal:
	case Articulation_Heel_To_Toe_Pedal:
	case Articulation_Open_String:
	case Articulation_Guitar_Lift:
	case Articulation_Guitar_Slide_Up:
	case Articulation_Guitar_Rip:
	case Articulation_Guitar_Fall_Off:
	case Articulation_Guitar_Slide_Down:
	case Articulation_Guitar_Spill:
	case Articulation_Guitar_Flip:
	case Articulation_Guitar_Smear:
	case Articulation_Guitar_Bend:
	case Articulation_Guitar_Doit:
	case Articulation_Guitar_Plop:
	case Articulation_Guitar_Wow_Wow:
	case Articulation_Guitar_Thumb:
	case Articulation_Guitar_Index_Finger:
	case Articulation_Guitar_Middle_Finger:
	case Articulation_Guitar_Ring_Finger:
	case Articulation_Guitar_Pinky_Finger:
	case Articulation_Guitar_Tap:
	case Articulation_Guitar_Hammer:
	case Articulation_Guitar_Pluck: {
		break;
	}
	default:
		break;
	}

	return affect;
}

bool Articulation::isTrill(ArticulationType type) {
	bool isTrill = false;

	switch (type) {
	case Articulation_Major_Trill:
	case Articulation_Minor_Trill:
	case Articulation_Trill_Section: {
		isTrill = true;
		break;
	}
	default:
		break;
	}

	return isTrill;
}

Articulation::XmlType Articulation::getXmlType() const {
	XmlType xmlType = Xml_Unknown;

	switch (type_) {
	case Articulation_Major_Trill:
	case Articulation_Minor_Trill:
	case Articulation_Trill_Section:
	case Articulation_Inverted_Short_Mordent:
	case Articulation_Inverted_Long_Mordent:
	case Articulation_Short_Mordent:
	case Articulation_Turn:
		//	case Articulation_Flat_Accidental_For_Trill :
		//	case Articulation_Sharp_Accidental_For_Trill :
		//	case Articulation_Natural_Accidental_For_Trill :
	case Articulation_Tremolo_Eighth:
	case Articulation_Tremolo_Sixteenth:
	case Articulation_Tremolo_Thirty_Second:
	case Articulation_Tremolo_Sixty_Fourth: {
		xmlType = Xml_Ornament;
		break;
	}
	case Articulation_Marcato:
	case Articulation_Marcato_Dot:
	case Articulation_Heavy_Attack:
	case Articulation_SForzando:
	case Articulation_SForzando_Inverted:
	case Articulation_SForzando_Dot:
	case Articulation_SForzando_Dot_Inverted:
	case Articulation_Heavier_Attack:
	case Articulation_Staccatissimo:
	case Articulation_Staccato:
	case Articulation_Tenuto:
	case Articulation_Pause:
	case Articulation_Grand_Pause: {
		xmlType = Xml_Articulation;
		break;
	}
	case Articulation_Up_Bow:
	case Articulation_Down_Bow:
	case Articulation_Up_Bow_Inverted:
	case Articulation_Down_Bow_Inverted:
	case Articulation_Natural_Harmonic:
	case Articulation_Artificial_Harmonic:
	case Articulation_Finger_1:
	case Articulation_Finger_2:
	case Articulation_Finger_3:
	case Articulation_Finger_4:
	case Articulation_Finger_5:
	case Articulation_Plus_Sign: {
		xmlType = Xml_Technical;
		break;
	}
	case Articulation_Arpeggio: {
		xmlType = Xml_Arpeggiate;
		break;
	}
	case Articulation_Fermata:
	case Articulation_Fermata_Inverted: {
		xmlType = Xml_Fermata;
		break;
	}
	case Articulation_Pedal_Down:
	case Articulation_Pedal_Up: {
		xmlType = Xml_Direction;
		break;
	}
		//	case Articulation_Toe_Pedal :
		//	case Articulation_Heel_Pedal :
		//	case Articulation_Toe_To_Heel_Pedal :
		//	case Articulation_Heel_To_Toe_Pedal :
		//	case Articulation_Open_String :
	default:
		break;
	}

	return xmlType;
}

///////////////////////////////////////////////////////////////////////////////
NoteContainer::NoteContainer() {
	musicDataType_ = MusicData_Note_Container;

	grace_ = false;
	cue_ = false;
	rest_ = false;
	raw_ = false;
	noteType_ = Note_Quarter;
	dot_ = 0;
	graceNoteType_ = Note_Eight;
	stemUp_ = true;
	showStem_ = true;
	stemLength_ = 7;
	inBeam_ = false;
	tuplet_ = 0;
	space_ = 2;//div by 0
	noteShift_ = 0;
}

NoteContainer::~NoteContainer(){
	for(unsigned int i=0; i<notes_.size(); ++i){
		delete notes_[i];
	}
	for(unsigned int i=0; i<articulations_.size(); ++i){
		delete articulations_[i];
	}
	notes_.clear();
	articulations_.clear();
}

void NoteContainer::setIsGrace(bool grace) {
	grace_ = grace;
}

bool NoteContainer::getIsGrace() const {
	return grace_;
}

void NoteContainer::setIsCue(bool cue) {
	cue_ = cue;
}

bool NoteContainer::getIsCue() const {
	return cue_;
}

void NoteContainer::setIsRest(bool rest) {
	rest_ = rest;
}

bool NoteContainer::getIsRest() const {
	return rest_;
}

void NoteContainer::setIsRaw(bool raw) {
	raw_ = raw;
}

bool NoteContainer::getIsRaw() const {
	return raw_;
}

void NoteContainer::setNoteType(NoteType type) {
	noteType_ = Note_Quarter;

	switch (type) {
	case Note_DoubleWhole:
	case Note_Whole:
	case Note_Half:
	case Note_Quarter:
	case Note_Eight:
	case Note_Sixteen:
	case Note_32:
	case Note_64:
	case Note_128:
	case Note_256: {
		noteType_ = type;
		break;
	}
	default: {
		break;
	}
	}
}

NoteType NoteContainer::getNoteType() const {
	return noteType_;
}

void NoteContainer::setDot(int dot) {
	dot_ = dot;
}

int NoteContainer::getDot() const {
	return dot_;
}

void NoteContainer::setGraceNoteType(NoteType type) {
	graceNoteType_ = type;
}

NoteType NoteContainer::getGraceNoteType() const {
	return graceNoteType_;
}

void NoteContainer::setInBeam(bool in) {
	inBeam_ = in;
}

bool NoteContainer::getInBeam() const {
	return inBeam_;
}

void NoteContainer::setStemUp(bool up) {
	stemUp_ = up;
}

bool NoteContainer::getStemUp(void) const {
	return stemUp_;
}

void NoteContainer::setShowStem(bool show) {
	showStem_ = show;
}

bool NoteContainer::getShowStem() const {
	return showStem_;
}

void NoteContainer::setStemLength(int line) {
	stemLength_ = line;
}

int NoteContainer::getStemLength() const {
	return stemLength_;
}

void NoteContainer::setTuplet(int tuplet) {
	tuplet_ = tuplet;
}

int NoteContainer::getTuplet() const {
	return tuplet_;
}

void NoteContainer::setSpace(int space) {
	space_ = space;
}

int NoteContainer::getSpace() const {
	return space_;
}

void NoteContainer::addNoteRest(Note* note) {
	notes_.push_back(note);
}

QList<Note*> NoteContainer::getNotesRests() const {
	return notes_;
}

void NoteContainer::addArticulation(Articulation* art) {
	articulations_.push_back(art);
}

QList<Articulation*> NoteContainer::getArticulations() const {
	return articulations_;
}

void NoteContainer::setNoteShift(int octave) {
	noteShift_ = octave;
}

int NoteContainer::getNoteShift() const {
	return noteShift_;
}

int NoteContainer::getOffsetStaff() const {
	if(getIsRest())
		return 0;

	int staffMove = 0;
	QList<OVE::Note*> notes = getNotesRests();
	for (unsigned int i = 0; i < notes.size(); ++i) {
		OVE::Note* notePtr = notes[i];
		staffMove = notePtr->getOffsetStaff();
	}

	return staffMove;
}

int NoteContainer::getDuration() const {
	int duration = (int) NoteDuration_4;

	switch (noteType_) {
	case Note_DoubleWhole: {
		duration = (int) NoteDuration_Double_Whole;
		break;
	}
	case Note_Whole: {
		duration = (int) NoteDuration_Whole;
		break;
	}
	case Note_Half: {
		duration = (int) NoteDuration_2;
		break;
	}
	case Note_Quarter: {
		duration = (int) NoteDuration_4;
		break;
	}
	case Note_Eight: {
		duration = (int) NoteDuration_8;
		break;
	}
	case Note_Sixteen: {
		duration = (int) NoteDuration_16;
		break;
	}
	case Note_32: {
		duration = (int) NoteDuration_32;
		break;
	}
	case Note_64: {
		duration = (int) NoteDuration_64;
		break;
	}
	case Note_128: {
		duration = (int) NoteDuration_128;
		break;
	}
	case Note_256: {
		duration = (int) NoteDuration_256;
		break;
	}
	default:
		break;
	}

	int dotLength = duration;

	for (int i = 0; i < dot_; ++i) {
		dotLength /= 2;
	}

	dotLength = duration - dotLength;

	duration += dotLength;

	return duration;
}

///////////////////////////////////////////////////////////////////////////////
Beam::Beam() {
	musicDataType_ = MusicData_Beam;
	grace_ = false;
}

void Beam::setIsGrace(bool grace) {
	grace_ = grace;
}

bool Beam::getIsGrace() const {
	return grace_;
}

void Beam::addLine(const MeasurePos& startMp, const MeasurePos& endMp) {
	lines_.push_back(qMakePair(startMp, endMp));
}

const QList<QPair<MeasurePos, MeasurePos> > Beam::getLines() const {
	return lines_;
}

///////////////////////////////////////////////////////////////////////////////
Tie::Tie() {
	musicDataType_ = MusicData_Tie;

	showOnTop_ = true;
	note_ = 72;
	height_ = 24;
}

void Tie::setShowOnTop(bool top) {
	showOnTop_ = top;
}

bool Tie::getShowOnTop() const {
	return showOnTop_;
}

void Tie::setNote(int note) {
	note_ = note;
}

int Tie::getNote() const {
	return note_;
}

void Tie::setHeight(int height) {
	height_ = height;
}

int Tie::getHeight() const {
	return height_;
}

///////////////////////////////////////////////////////////////////////////////
Glissando::Glissando() {
	musicDataType_ = MusicData_Glissando;

	straight_ = true;
	text_ = "gliss.";
	lineThick_ = 8;
}

void Glissando::setStraightWavy(bool straight) {
	straight_ = straight;
}

bool Glissando::getStraightWavy() const {
	return straight_;
}

void Glissando::setText(const QString& text) {
	text_ = text;
}

QString Glissando::getText() const {
	return text_;
}

void Glissando::setLineThick(int thick) {
	lineThick_ = thick;
}

int Glissando::getLineThick() const {
	return lineThick_;
}

///////////////////////////////////////////////////////////////////////////////
Decorator::Decorator() :
	decoratorType_(Decorator_Articulation),
	artType_(Articulation_Marcato) {
	musicDataType_ = MusicData_Decorator;
}

void Decorator::setDecoratorType(DecoratorType type) {
	decoratorType_ = type;
}

Decorator::DecoratorType Decorator::getDecoratorType() const {
	return decoratorType_;
}

void Decorator::setArticulationType(ArticulationType type) {
	artType_ = type;
}

ArticulationType Decorator::getArticulationType() const {
	return artType_;
}

///////////////////////////////////////////////////////////////////////////////
MeasureRepeat::MeasureRepeat() {
	musicDataType_ = MusicData_Measure_Repeat;
	singleRepeat_ = true;
}

void MeasureRepeat::setSingleRepeat(bool single) {
	singleRepeat_ = single;

	start()->setMeasure(0);
	start()->setOffset(0);
	stop()->setMeasure(single ? 1 : 2);
	stop()->setOffset(0);
}

bool MeasureRepeat::getSingleRepeat() const {
	return singleRepeat_;
}

///////////////////////////////////////////////////////////////////////////////
Tuplet::Tuplet() :
	tuplet_(3), space_(2), height_(0), noteType_(Note_Quarter){
	musicDataType_ = MusicData_Tuplet;
	mark_ = new OffsetElement();
}

Tuplet::~Tuplet(){
	delete mark_;
}

void Tuplet::setTuplet(int tuplet) {
	tuplet_ = tuplet;
}

int Tuplet::getTuplet() const {
	return tuplet_;
}

void Tuplet::setSpace(int space) {
	space_ = space;
}

int Tuplet::getSpace() const {
	return space_;
}

OffsetElement* Tuplet::getMarkHandle() const {
	return mark_;
}

void Tuplet::setHeight(int height) {
	height_ = height;
}

int Tuplet::getHeight() const {
	return height_;
}

void Tuplet::setNoteType(NoteType type) {
	noteType_ = type;
}

NoteType Tuplet::getNoteType() const {
	return noteType_;
}

///////////////////////////////////////////////////////////////////////////////
Harmony::Harmony() {
	musicDataType_ = MusicData_Harmony;

	harmonyType_ = Harmony_maj;
	root_ = 0;
	bass_ = -1;//0xff
	bassOnBottom_ = false;
	angle_ = 0;
}

void Harmony::setHarmonyType(HarmonyType type) {
	harmonyType_ = type;
}

HarmonyType Harmony::getHarmonyType() const {
	return harmonyType_;
}

void Harmony::setRoot(int root) {
	root_ = root;
}

int Harmony::getRoot() const {
	return root_;
}

void Harmony::setBass(int bass) {
	bass_ = bass;
}

int Harmony::getBass() const {
	return bass_;
}

void Harmony::setBassOnBottom(bool on) {
	bassOnBottom_ = on;
}

bool Harmony::getBassOnBottom() const {
	return bassOnBottom_;
}

void Harmony::setAngle(int angle) {
	angle_ = angle;
}

int Harmony::getAngle() const {
	return angle_;
}

///////////////////////////////////////////////////////////////////////////////
Clef::Clef() {
	musicDataType_ = MusicData_Clef;

	clefType_ = Clef_Treble;
}

void Clef::setClefType(int type) {
	clefType_ = (ClefType) type;
}

ClefType Clef::getClefType() const {
	return clefType_;
}

///////////////////////////////////////////////////////////////////////////////
Lyric::Lyric() {
	musicDataType_ = MusicData_Lyric;

	lyric_ = QString();
	verse_ = 0;
}

void Lyric::setLyric(const QString& lyricText) {
	lyric_ = lyricText;
}

QString Lyric::getLyric() const {
	return lyric_;
}

void Lyric::setVerse(int verse) {
	verse_ = verse;
}

int Lyric::getVerse() const {
	return verse_;
}

///////////////////////////////////////////////////////////////////////////////
Slur::Slur() {
	musicDataType_ = MusicData_Slur;

	containerCount_ = 1;
	showOnTop_ = true;
	noteTimePercent_ = 100;

	handle_2_ = new OffsetElement();
	handle_3_ = new OffsetElement();
}

Slur::~Slur() {
	delete handle_2_;
	delete handle_3_;
}

void Slur::setContainerCount(int count) {
	containerCount_ = count;
}

int Slur::getContainerCount() const {
	return containerCount_;
}

void Slur::setShowOnTop(bool top) {
	showOnTop_ = top;
}

bool Slur::getShowOnTop() const {
	return showOnTop_;
}

OffsetElement* Slur::getHandle2() const {
	return handle_2_;
}

OffsetElement* Slur::getHandle3() const {
	return handle_3_;
}

void Slur::setNoteTimePercent(int percent) {
	noteTimePercent_ = percent;
}

int Slur::getNoteTimePercent() const {
	return noteTimePercent_;
}

///////////////////////////////////////////////////////////////////////////////
Dynamics::Dynamics() {
	musicDataType_ = MusicData_Dynamics;

	dynamicsType_ = Dynamics_pppp;
	playback_ = true;
	velocity_ = 30;
}

void Dynamics::setDynamicsType(int type) {
	dynamicsType_ = (DynamicsType) type;
}

DynamicsType Dynamics::getDynamicsType() const {
	return dynamicsType_;
}

void Dynamics::setIsPlayback(bool play) {
	playback_ = play;
}

bool Dynamics::getIsPlayback() const {
	return playback_;
}

void Dynamics::setVelocity(int vel) {
	velocity_ = vel;
}

int Dynamics::getVelocity() const {
	return velocity_;
}

///////////////////////////////////////////////////////////////////////////////
WedgeEndPoint::WedgeEndPoint() {
	musicDataType_ = MusicData_Wedge_EndPoint;

	wedgeType_ = Wedge_Cres;
	height_ = 24;
	wedgeStart_ = true;
}

void WedgeEndPoint::setWedgeType(WedgeType type) {
	wedgeType_ = type;
}

WedgeType WedgeEndPoint::getWedgeType() const {
	return wedgeType_;
}

void WedgeEndPoint::setHeight(int height) {
	height_ = height;
}

int WedgeEndPoint::getHeight() const {
	return height_;
}

void WedgeEndPoint::setWedgeStart(bool wedgeStart) {
	wedgeStart_ = wedgeStart;
}

bool WedgeEndPoint::getWedgeStart() const {
	return wedgeStart_;
}

///////////////////////////////////////////////////////////////////////////////
Wedge::Wedge() {
	musicDataType_ = MusicData_Wedge;

	wedgeType_ = Wedge_Cres;
	height_ = 24;
}

void Wedge::setWedgeType(WedgeType type) {
	wedgeType_ = type;
}

WedgeType Wedge::getWedgeType() const {
	return wedgeType_;
}

void Wedge::setHeight(int height) {
	height_ = height;
}

int Wedge::getHeight() const {
	return height_;
}

///////////////////////////////////////////////////////////////////////////////
Pedal::Pedal() {
	musicDataType_ = MusicData_Pedal;

	half_ = false;
	playback_ = false;
	playOffset_ = 0;

	pedalHandle_ = new OffsetElement();
}

Pedal::~Pedal() {
	delete pedalHandle_;
}

void Pedal::setHalf(bool half) {
	half_ = half;
}

bool Pedal::getHalf() const {
	return half_;
}

OffsetElement* Pedal::getPedalHandle() const {
	return pedalHandle_;
}

void Pedal::setIsPlayback(bool playback) {
	playback_ = playback;
}

bool Pedal::getIsPlayback() const {
	return playback_;
}

void Pedal::setPlayOffset(int offset) {
	playOffset_ = offset;
}

int Pedal::getPlayOffset() const {
	return playOffset_;
}

///////////////////////////////////////////////////////////////////////////////
KuoHao::KuoHao() {
	musicDataType_ = MusicData_KuoHao;

	kuohaoType_ = KuoHao_Parentheses;
	height_ = 0;
}

void KuoHao::setHeight(int height) {
	height_ = height;
}

int KuoHao::getHeight() const {
	return height_;
}

void KuoHao::setKuohaoType(int type) {
	kuohaoType_ = (KuoHaoType) type;
}

KuoHaoType KuoHao::getKuohaoType() const {
	return kuohaoType_;
}

///////////////////////////////////////////////////////////////////////////////
Expressions::Expressions() {
	musicDataType_ = MusicData_Expressions;

	text_ = QString();
}

void Expressions::setText(const QString& str) {
	text_ = str;
}

QString Expressions::getText() const {
	return text_;
}

///////////////////////////////////////////////////////////////////////////////
HarpPedal::HarpPedal() :
	showType_(0),
	showCharFlag_(0) {
	musicDataType_ = MusicData_Harp_Pedal;
}

void HarpPedal::setShowType(int type) {
	showType_ = type;
}

int HarpPedal::getShowType() const {
	return showType_;
}

void HarpPedal::setShowCharFlag(int flag) {
	showCharFlag_ = flag;
}

int HarpPedal::getShowCharFlag() const {
	return showCharFlag_;
}

///////////////////////////////////////////////////////////////////////////////
OctaveShift::OctaveShift() :
	octaveShiftType_(OctaveShift_8),
	octaveShiftPosition_(OctavePosition_Start),
	endTick_(0) {
	musicDataType_ = MusicData_OctaveShift;
}

void OctaveShift::setOctaveShiftType(int type) {
	octaveShiftType_ = (OctaveShiftType) type;
}

OctaveShiftType OctaveShift::getOctaveShiftType() const {
	return octaveShiftType_;
}

int OctaveShift::getNoteShift() const {
	int shift = 12;

	switch (getOctaveShiftType()) {
	case OctaveShift_8: {
		shift = 12;
		break;
	}
	case OctaveShift_Minus_8: {
		shift = -12;
		break;
	}
	case OctaveShift_15: {
		shift = 24;
		break;
	}
	case OctaveShift_Minus_15: {
		shift = -24;
		break;
	}
	default:
		break;
	}

	return shift;
}

void OctaveShift::setEndTick(int tick) {
	endTick_ = tick;
}

int OctaveShift::getEndTick() const {
	return endTick_;
}

///////////////////////////////////////////////////////////////////////////////
OctaveShiftEndPoint::OctaveShiftEndPoint() {
	musicDataType_ = MusicData_OctaveShift_EndPoint;

	octaveShiftType_ = OctaveShift_8;
	octaveShiftPosition_ = OctavePosition_Start;
	endTick_ = 0;
}

void OctaveShiftEndPoint::setOctaveShiftType(int type) {
	octaveShiftType_ = (OctaveShiftType) type;
}

OctaveShiftType OctaveShiftEndPoint::getOctaveShiftType() const {
	return octaveShiftType_;
}

void OctaveShiftEndPoint::setOctaveShiftPosition(int position) {
	octaveShiftPosition_ = (OctaveShiftPosition) position;
}

OctaveShiftPosition OctaveShiftEndPoint::getOctaveShiftPosition() const {
	return octaveShiftPosition_;
}

void OctaveShiftEndPoint::setEndTick(int tick) {
	endTick_ = tick;
}

int OctaveShiftEndPoint::getEndTick() const {
	return endTick_;
}

///////////////////////////////////////////////////////////////////////////////
MultiMeasureRest::MultiMeasureRest() {
	musicDataType_ = MusicData_Multi_Measure_Rest;
	measureCount_ = 0;
}

void MultiMeasureRest::setMeasureCount(int count) {
	measureCount_ = count;
}

int MultiMeasureRest::getMeasureCount() const {
	return measureCount_;
}

///////////////////////////////////////////////////////////////////////////////
Tempo::Tempo() {
	musicDataType_ = MusicData_Tempo;

	leftNoteType_ = 3;
	showMark_ = false;
	showText_ = false;
	showParenthesis_ = false;
	typeTempo_ = 96;
	leftText_ = QString();
	rightText_ = QString();
	swingEighth_ = false;
	rightNoteType_ = 3;
}

void Tempo::setLeftNoteType(int type) {
	leftNoteType_ = type;
}

NoteType Tempo::getLeftNoteType() const {
	return (NoteType) leftNoteType_;
}

void Tempo::setShowMark(bool show) {
	showMark_ = show;
}

bool Tempo::getShowMark() const {
	return showMark_;
}

void Tempo::setShowBeforeText(bool show) {
	showText_ = show;
}

bool Tempo::getShowBeforeText() const {
	return showText_;
}

void Tempo::setShowParenthesis(bool show) {
	showParenthesis_ = show;
}

bool Tempo::getShowParenthesis() const {
	return showParenthesis_;
}

void Tempo::setTypeTempo(int tempo) {
	typeTempo_ = tempo;
}

int Tempo::getTypeTempo() const {
	return typeTempo_;
}

int Tempo::getQuarterTempo() const {
	double factor = pow(2.0, (int) Note_Quarter - (int) getLeftNoteType());
	int tempo = int((double) getTypeTempo() * factor);

	return tempo;
}

void Tempo::setLeftText(const QString& str) {
	leftText_ = str;
}

QString Tempo::getLeftText() const {
	return leftText_;
}

void Tempo::setRightText(const QString& str) {
	rightText_ = str;
}

QString Tempo::getRightText() const {
	return rightText_;
}

void Tempo::setSwingEighth(bool swing) {
	swingEighth_ = swing;
}

bool Tempo::getSwingEighth() const {
	return swingEighth_;
}

void Tempo::setRightNoteType(int type) {
	rightNoteType_ = type;
}

int Tempo::getRightNoteType() const {
	return rightNoteType_;
}

///////////////////////////////////////////////////////////////////////////////
Text::Text() {
	musicDataType_ = MusicData_Text;

	textType_ = Text_Rehearsal;
	horiMargin_ = 8;
	vertMargin_ = 8;
	lineThick_ = 4;
	text_ = QString();
	width_ = 0;
	height_ = 0;
}

void Text::setTextType(TextType type) {
	textType_ = type;
}

Text::TextType Text::getTextType() const {
	return textType_;
}

void Text::setHorizontalMargin(int margin) {
	horiMargin_ = margin;
}

int Text::getHorizontalMargin() const {
	return horiMargin_;
}

void Text::setVerticalMargin(int margin) {
	vertMargin_ = margin;
}

int Text::getVerticalMargin() const {
	return vertMargin_;
}

void Text::setLineThick(int thick) {
	lineThick_ = thick;
}

int Text::getLineThick() const {
	return lineThick_;
}

void Text::setText(const QString& text) {
	text_ = text;
}

QString Text::getText() const {
	return text_;
}

void Text::setWidth(int width) {
	width_ = width;
}

int Text::getWidth() const {
	return width_;
}

void Text::setHeight(int height) {
	height_ = height;
}

int Text::getHeight() const {
	return height_;
}

///////////////////////////////////////////////////////////////////////////////
TimeSignature::TimeSignature() {
	numerator_ = 4;
	denominator_ = 4;
	isSymbol_ = false;
	beatLength_ = 480;
	barLength_ = 1920;
	barLengthUnits_ = 0x400;
	replaceFont_ = false;
	showBeatGroup_ = false;

	groupNumerator1_ = 0;
	groupNumerator2_ = 0;
	groupNumerator3_ = 0;
	groupDenominator1_ = 4;
	groupDenominator2_ = 4;
	groupDenominator3_ = 4;

	beamGroup1_ = 4;
	beamGroup2_ = 0;
	beamGroup3_ = 0;
	beamGroup4_ = 0;

	beamCount16th_ = 4;
	beamCount32th_ = 1;
}

void TimeSignature::setNumerator(int numerator) {
	numerator_ = numerator;
}

int TimeSignature::getNumerator() const {
	return numerator_;
}

void TimeSignature::setDenominator(int denominator) {
	denominator_ = denominator;
}

int TimeSignature::getDenominator() const {
	return denominator_;
}

void TimeSignature::setIsSymbol(bool symbol) {
	isSymbol_ = symbol;
}

bool TimeSignature::getIsSymbol() const {
	if (numerator_ == 2 && denominator_ == 2) {
		return true;
	}

	return isSymbol_;
}

void TimeSignature::setBeatLength(int length) {
	beatLength_ = length;
}

int TimeSignature::getBeatLength() const {
	return beatLength_;
}

void TimeSignature::setBarLength(int length) {
	barLength_ = length;
}

int TimeSignature::getBarLength() const {
	return barLength_;
}

void TimeSignature::addBeat(int startUnit, int lengthUnit, int startTick) {
	BeatNode node;
	node.startUnit_ = startUnit;
	node.lengthUnit_ = lengthUnit;
	node.startTick_ = startTick;
	beats_.push_back(node);
}

void TimeSignature::endAddBeat() {
	unsigned int i;
	barLengthUnits_ = 0;

	for (i = 0; i < beats_.size(); ++i) {
		barLengthUnits_ += beats_[i].lengthUnit_;
	}
}

int TimeSignature::getUnits() const {
	return barLengthUnits_;
}

void TimeSignature::setReplaceFont(bool replace) {
	replaceFont_ = replace;
}

bool TimeSignature::getReplaceFont() const {
	return replaceFont_;
}

void TimeSignature::setShowBeatGroup(bool show) {
	showBeatGroup_ = show;
}

bool TimeSignature::getShowBeatGroup() const {
	return showBeatGroup_;
}

void TimeSignature::setGroupNumerator1(int numerator) {
	groupNumerator1_ = numerator;
}

void TimeSignature::setGroupNumerator2(int numerator) {
	groupNumerator2_ = numerator;
}

void TimeSignature::setGroupNumerator3(int numerator) {
	groupNumerator3_ = numerator;
}

void TimeSignature::setGroupDenominator1(int denominator) {
	groupDenominator1_ = denominator;
}

void TimeSignature::setGroupDenominator2(int denominator) {
	groupDenominator2_ = denominator;
}

void TimeSignature::setGroupDenominator3(int denominator) {
	groupDenominator3_ = denominator;
}

void TimeSignature::setBeamGroup1(int count) {
	beamGroup1_ = count;
}

void TimeSignature::setBeamGroup2(int count) {
	beamGroup2_ = count;
}

void TimeSignature::setBeamGroup3(int count) {
	beamGroup3_ = count;
}

void TimeSignature::setBeamGroup4(int count) {
	beamGroup4_ = count;
}

void TimeSignature::set16thBeamCount(int count) {
	beamCount16th_ = count;
}

void TimeSignature::set32thBeamCount(int count) {
	beamCount32th_ = count;
}

///////////////////////////////////////////////////////////////////////////////
Key::Key() {
	key_ = 0;
	set_ = false;
	previousKey_ = 0;
	symbolCount_ = 0;
}

void Key::setKey(int key) {
	key_ = key;
	set_ = true;
}

int Key::getKey() const {
	return key_;
}

bool Key::getSetKey() const {
	return set_;
}

void Key::setPreviousKey(int key) {
	previousKey_ = key;
}

int Key::getPreviousKey() const {
	return previousKey_;
}

void Key::setSymbolCount(int count) {
	symbolCount_ = count;
}

int Key::getSymbolCount() const {
	return symbolCount_;
}

///////////////////////////////////////////////////////////////////////////////
RepeatSymbol::RepeatSymbol() :
	text_("#1"), repeatType_(Repeat_Segno) {
	musicDataType_ = MusicData_Repeat;
}

void RepeatSymbol::setText(const QString& text) {
	text_ = text;
}

QString RepeatSymbol::getText() const {
	return text_;
}

void RepeatSymbol::setRepeatType(int repeatType) {
	repeatType_ = (RepeatType) repeatType;
}

RepeatType RepeatSymbol::getRepeatType() const {
	return repeatType_;
}

///////////////////////////////////////////////////////////////////////////////
NumericEnding::NumericEnding() {
	musicDataType_ = MusicData_Numeric_Ending;

	height_ = 0;
	text_ = QString();
	numericHandle_ = new OffsetElement();
}

NumericEnding::~NumericEnding() {
	delete numericHandle_;
}

OffsetElement* NumericEnding::getNumericHandle() const {
	return numericHandle_;
}

void NumericEnding::setHeight(int height) {
	height_ = height;
}

int NumericEnding::getHeight() const {
	return height_;
}

void NumericEnding::setText(const QString& text) {
	text_ = text;
}

QString NumericEnding::getText() const {
	return text_;
}

QList<int> NumericEnding::getNumbers() const {
	unsigned int i;
	QStringList strs = text_.split(",", QString::SkipEmptyParts);
	QList<int> endings;

	for (i = 0; i < strs.size(); ++i) {
		bool ok;
		int num = strs[i].toInt(&ok);
		endings.push_back(num);
	}

	return endings;
}

int NumericEnding::getJumpCount() const {
	QList<int> numbers = getNumbers();
	int count = 0;

	for (unsigned int i = 0; i < numbers.size(); ++i) {
		if ((int)i + 1 != numbers[i]) {
			break;
		}

		count = i + 1;
	}

	return count;
}

///////////////////////////////////////////////////////////////////////////////
BarNumber::BarNumber() {
	index_ = 0;
	showOnParagraphStart_ = false;
	align_ = 0;
	showFlag_ = 1; // staff
	barRange_ = 1; // can't be 0
	prefix_ = QString();
}

void BarNumber::setIndex(int index) {
	index_ = index;
}

int BarNumber::getIndex() const {
	return index_;
}

void BarNumber::setShowOnParagraphStart(bool show) {
	showOnParagraphStart_ = show;
}

bool BarNumber::getShowOnParagraphStart() const {
	return showOnParagraphStart_;
}

void BarNumber::setAlign(int align)// 0:left, 1:center, 2:right
{
	align_ = align;
}

int BarNumber::getAlign() const {
	return align_;
}

void BarNumber::setShowFlag(int flag) {
	showFlag_ = flag;
}

int BarNumber::getShowFlag() const {
	return showFlag_;
}

void BarNumber::setShowEveryBarCount(int count) {
	barRange_ = count;
}

int BarNumber::getShowEveryBarCount() const {
	return barRange_;
}

void BarNumber::setPrefix(const QString& str) {
	prefix_ = str;
}

QString BarNumber::getPrefix() const {
	return prefix_;
}

///////////////////////////////////////////////////////////////////////////////
MidiController::MidiController() {
	midiType_ = Midi_Controller;
	controller_ = 64; // pedal
	value_ = 0;
}

void MidiController::setController(int number) {
	controller_ = number;
}

int MidiController::getController() const {
	return controller_;
}

void MidiController::setValue(int value) {
	value_ = value;
}

int MidiController::getValue() const {
	return value_;
}

///////////////////////////////////////////////////////////////////////////////
MidiProgramChange::MidiProgramChange() {
	midiType_ = Midi_Program_Change;
	patch_ = 0; // grand piano
}

void MidiProgramChange::setPatch(int patch) {
	patch_ = patch;
}

int MidiProgramChange::getPatch() const {
	return patch_;
}

///////////////////////////////////////////////////////////////////////////////
MidiChannelPressure::MidiChannelPressure() :
	pressure_(0) {
	midiType_ = Midi_Channel_Pressure;
}

void MidiChannelPressure::setPressure(int pressure) {
	pressure_ = pressure;
}

int MidiChannelPressure::getPressure() const {
	return pressure_;
}

///////////////////////////////////////////////////////////////////////////////
MidiPitchWheel::MidiPitchWheel() {
	midiType_ = Midi_Pitch_Wheel;
	value_ = 0;
}

void MidiPitchWheel::setValue(int value) {
	value_ = value;
}

int MidiPitchWheel::getValue() const {
	return value_;
}

///////////////////////////////////////////////////////////////////////////////
Measure::Measure(int index) {
	barNumber_ = new BarNumber();
	barNumber_->setIndex(index);
	time_ = new TimeSignature();

	clear();
}

Measure::~Measure(){
	clear();

	delete barNumber_;
	delete time_;
}

BarNumber* Measure::getBarNumber() const {
	return barNumber_;
}

TimeSignature* Measure::getTime() const {
	return time_;
}

void Measure::setLeftBarline(int barline) {
	leftBarline_ = (BarlineType) barline;
}

BarlineType Measure::getLeftBarline() const {
	return leftBarline_;
}

void Measure::setRightBarline(int barline) {
	rightBarline_ = (BarlineType) barline;
}

BarlineType Measure::getRightBarline() const {
	return rightBarline_;
}

void Measure::setBackwardRepeatCount(int repeatCount) {
	repeatCount_ = repeatCount;
}

int Measure::getBackwardRepeatCount() const {
	return repeatCount_;
}

void Measure::setTypeTempo(double tempo) {
	typeTempo_ = tempo;
}

double Measure::getTypeTempo() const {
	return typeTempo_;
}

void Measure::setIsPickup(bool pickup) {
	pickup_ = pickup;
}

bool Measure::getIsPickup() const {
	return pickup_;
}

void Measure::setIsMultiMeasureRest(bool rest) {
	multiMeasureRest_ = rest;
}

bool Measure::getIsMultiMeasureRest() const {
	return multiMeasureRest_;
}

void Measure::setMultiMeasureRestCount(int count) {
	multiMeasureRestCount_ = count;
}

int Measure::getMultiMeasureRestCount() const {
	return multiMeasureRestCount_;
}

void Measure::clear() {
	leftBarline_ = Barline_Default;
	rightBarline_ = Barline_Default;
	repeatCount_ = 1;
	typeTempo_ = 96.00;
	setLength(0x780); //time = 4/4
	pickup_ = false;
	multiMeasureRest_ = false;
	multiMeasureRestCount_ = 0;
}

///////////////////////////////////////////////////////////////////////////////
MeasureData::MeasureData() {
	key_ = new Key();
	clef_ = new Clef();
}

MeasureData::~MeasureData(){
	unsigned int i;
	for(i=0; i<musicDatas_.size(); ++i){
		delete musicDatas_[i];
	}
	musicDatas_.clear();

	// noteContainers_ also in musicDatas_, no need to destory
	noteContainers_.clear();

	// only delete at element start
	for(i=0; i<crossMeasureElements_.size(); ++i){
		if(crossMeasureElements_[i].second){
			delete crossMeasureElements_[i].first;
		}
	}
	crossMeasureElements_.clear();

	for(i=0; i<midiDatas_.size(); ++i){
		delete midiDatas_[i];
	}
	midiDatas_.clear();

	delete key_;
	delete clef_;
}

Key* MeasureData::getKey() const {
	return key_;
}

Clef* MeasureData::getClef() const {
	return clef_;
}

void MeasureData::addNoteContainer(NoteContainer* ptr) {
	noteContainers_.push_back(ptr);
}

QList<NoteContainer*> MeasureData::getNoteContainers() const {
	return noteContainers_;
}

void MeasureData::addMusicData(MusicData* ptr) {
	musicDatas_.push_back(ptr);
}

QList<MusicData*> MeasureData::getMusicDatas(MusicDataType type) {
	unsigned int i;
	QList<MusicData*> notations;

	for (i = 0; i < musicDatas_.size(); ++i) {
		if (type == MusicData_None || musicDatas_[i]->getMusicDataType() == type) {
			notations.push_back(musicDatas_[i]);
		}
	}

	return notations;
}

void MeasureData::addCrossMeasureElement(MusicData* ptr, bool start) {
	crossMeasureElements_.push_back(qMakePair(ptr, start));
}

QList<MusicData*> MeasureData::getCrossMeasureElements(
		MusicDataType type, PairType pairType) {
	unsigned int i;
	QList<MusicData*> pairs;

	for (i = 0; i < crossMeasureElements_.size(); ++i) {
		if ((type == MusicData_None || crossMeasureElements_[i].first->getMusicDataType() == type)
				&& (pairType == PairType_All || ((crossMeasureElements_[i].second && pairType == PairType_Start)
						|| (!crossMeasureElements_[i].second && pairType == PairType_Stop)))) {
			pairs.push_back(crossMeasureElements_[i].first);
		}
	}

	return pairs;
}

void MeasureData::addMidiData(MidiData* ptr) {
	midiDatas_.push_back(ptr);
}

QList<MidiData*> MeasureData::getMidiDatas(MidiType type) {
	unsigned int i;
	QList<MidiData*> datas;

	for (i = 0; i < midiDatas_.size(); ++i) {
		if (type == Midi_None || midiDatas_[i]->getMidiType() == type) {
			datas.push_back(midiDatas_[i]);
		}
	}

	return datas;
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
StreamHandle::StreamHandle() :
	size_(0), curPos_(0), point_(NULL) {
}

StreamHandle::StreamHandle(unsigned char* p, int size) :
	size_(size), curPos_(0), point_(p) {
}

StreamHandle::~StreamHandle() {
	point_ = NULL;
}

bool StreamHandle::read(char* buff, int size) {
	if (point_ != NULL && curPos_ + size <= size_) {
		memcpy(buff, point_ + curPos_, size);
		curPos_ += size;

		return true;
	}

	return false;
}

bool StreamHandle::write(char* /*buff*/, int /*size*/) {
	return true;
}

// Block.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////
Block::Block() {
	doResize(0);
}

Block::Block(unsigned int count) {
	doResize(count);
}

void Block::resize(unsigned int count) {
	doResize(count);
}

void Block::doResize(unsigned int count) {
	data_.clear();
	for(unsigned int i=0; i<count; ++i) {
		data_.push_back('\0');
	}
	//data_.resize(count);
}

const unsigned char* Block::data() const {
	//return const_cast<unsigned char*>(&data_.front());
	return &data_.front();
}

unsigned char* Block::data() {
	return &data_.front();
}

unsigned int Block::size() const {
	return data_.size();
}

bool Block::toBoolean() const {
	if (data() == NULL) {
		return false;
	}

	return size() == 1 && data()[0] == 0x01;
}

unsigned int Block::toUnsignedInt() const {
	if (data() == NULL) {
		return 0;
	}

	unsigned int i;
	unsigned int num(0);

	for (i = 0; i < sizeof(unsigned int) && i < size(); ++i) {
		num = (num << 8) + *(data() + i);
	}

	return num;
}

int Block::toInt() const {
	if (data() == NULL) {
		return 0;
	}

	unsigned int i;
	int num = 0;

	for (i = 0; i < sizeof(unsigned int) && i < size(); ++i) {
		num = (num << 8) + (int) *(data() + i);
	}

	std::size_t minSize = sizeof(unsigned int);
	if (size() < minSize) {
		minSize = size();
	}

	if ((*(data()) & 0x80) == 0x80) {
		int maxNum = int(pow(2.0, (int) minSize * 8));
		num -= maxNum;
		//num *= -1;
	}

	return num;
}

QByteArray Block::toStrByteArray() const {
	if (data() == NULL) {
		return QByteArray();
	}

	QByteArray arr((char*) data(), size());

	return arr;
}

QByteArray Block::fixedSizeBufferToStrByteArray() const {
	unsigned int i;
	QByteArray str;

	for (i = 0; i < size(); ++i) {
		if (*(data() + i) == '\0') {
			break;
		}

		str += (char) *(data() + i);
	}

	return str;
}

bool Block::operator ==(const Block& block) const {
	unsigned int i;

	if (size() != block.size()) {
		return false;
	}

	for (i = 0; i < size() && i < block.size(); ++i) {
		if (*(data() + i) != *(block.data() + i)) {
			return false;
		}
	}

	return true;
}

bool Block::operator !=(const Block& block) const {
	return !(*this == block);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
FixedBlock::FixedBlock() :
	Block() {
}

FixedBlock::FixedBlock(unsigned int count) :
	Block(count) {
}

void FixedBlock::resize(unsigned int /*count*/) {
	//	Block::resize(size);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SizeBlock::SizeBlock() :
	FixedBlock(4) {
}

unsigned int SizeBlock::toSize() const {
	unsigned int i;
	unsigned int num(0);
	const unsigned int SIZE = 4;

	for (i = 0; i < SIZE; ++i) {
		num = (num << 8) + *(data() + i);
	}

	return num;
}

/*void SizeBlock::fromUnsignedInt(unsigned int count)
 {
 unsigned_int_to_char_buffer(count, data());
 }*/

///////////////////////////////////////////////////////////////////////////////////////////////////
NameBlock::NameBlock() :
	FixedBlock(4) {
}

/*void NameBlock::setValue(const char* const name)
 {
 unsigned int i;

 for( i=0; i<size() && *(name+i)!='\0'; ++i )
 {
 *(data()+i) = *(name+i);
 }
 }*/

bool NameBlock::isEqual(const QString& name) const {
	unsigned int i;

	if (name.size() != size()) {
		return false;
	}

	for (i = 0; i < size() && name.size(); ++i) {
		if (data()[i] != name[i]) {
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CountBlock::CountBlock() :
	FixedBlock(2) {
}

/*void CountBlock::setValue(unsigned short count)
 {
 unsigned int i;
 unsigned int SIZE = sizeof(unsigned short);

 for( i=0; i<SIZE; ++i )
 {
 data()[SIZE-1-i] = count % 256;
 count /= 256;
 }
 }*/

unsigned short CountBlock::toCount() const {
	unsigned int i;
	unsigned short num = 0;

	for (i = 0; i < size() && i < sizeof(unsigned short); ++i) {
		num = (num << 8) + *(data() + i);
	}

	return num;
}

// Chunk.cpp
const QString Chunk::TrackName	= "TRAK";
const QString Chunk::PageName	= "PAGE";
const QString Chunk::LineName	= "LINE";
const QString Chunk::StaffName	= "STAF";
const QString Chunk::MeasureName = "MEAS";
const QString Chunk::ConductName = "COND";
const QString Chunk::BdatName	= "BDAT";

Chunk::Chunk() {
}

NameBlock Chunk::getName() const {
	return nameBlock_;
}

////////////////////////////////////////////////////////////////////////////////
const unsigned int SizeChunk::version3TrackSize = 0x13a;

SizeChunk::SizeChunk() :
	Chunk() {
	sizeBlock_ = new SizeBlock();
	dataBlock_ = new Block();
}

SizeChunk::~SizeChunk() {
	delete sizeBlock_;
	delete dataBlock_;
}

SizeBlock* SizeChunk::getSizeBlock() const {
	return sizeBlock_;
}

Block* SizeChunk::getDataBlock() const {
	return dataBlock_;
}

/////////////////////////////////////////////////////////////////////////////////
GroupChunk::GroupChunk() : Chunk() {
	childCount_ = new CountBlock();
}

GroupChunk::~GroupChunk() {
	delete childCount_;
}

CountBlock* GroupChunk::getCountBlock() const {
	return childCount_;
}

// ChunkParse.cpp
unsigned int getHighNibble(unsigned int byte) {
	return byte / 16;
}

unsigned int getLowNibble(unsigned int byte) {
	return byte % 16;
}

int oveKeyToKey(int oveKey) {
	int key = 0;

	if( oveKey == 0 ) {
		key = 0;
	}
	else if( oveKey > 7 ) {
		key = oveKey - 7;
	}
	else if( oveKey <= 7 ) {
		key = oveKey * (-1);
	}

	return key;
}

///////////////////////////////////////////////////////////////////////////////
BasicParse::BasicParse(OveSong* ove) :
	ove_(ove), handle_(NULL), notify_(NULL) {
}

BasicParse::BasicParse() :
	ove_(NULL), handle_(NULL), notify_(NULL) {
}

BasicParse::~BasicParse() {
	ove_ = NULL;
	handle_ = NULL;
	notify_ = NULL;
}

void BasicParse::setNotify(IOveNotify* notify) {
	notify_ = notify;
}

bool BasicParse::parse() {
	return false;
}

bool BasicParse::readBuffer(Block& placeHolder, unsigned int size) {
	if (handle_ == NULL) {
		return false;
	}
	if (placeHolder.size() != size) {
		placeHolder.resize(size);
	}

	if (size > 0) {
		return handle_->read((char*) placeHolder.data(), placeHolder.size());
	}

	return true;
}

bool BasicParse::jump(int offset) {
	if (handle_ == NULL || offset < 0) {
		return false;
	}

	if (offset > 0) {
		Block placeHolder(offset);
		return handle_->read((char*) placeHolder.data(), placeHolder.size());
	}

	return true;
}

void BasicParse::messageOut(const QString& str) {
	if (notify_ != NULL) {
		notify_->loadInfo(str);
	}
}

///////////////////////////////////////////////////////////////////////////////
OvscParse::OvscParse(OveSong* ove) :
	BasicParse(ove), chunk_(NULL) {
}

OvscParse::~OvscParse() {
	chunk_ = NULL;
}

void OvscParse::setOvsc(SizeChunk* chunk) {
	chunk_ = chunk;
}

bool OvscParse::parse() {
	Block* dataBlock = chunk_->getDataBlock();
	unsigned int blockSize = chunk_->getSizeBlock()->toSize();
	StreamHandle handle(dataBlock->data(), blockSize);
	Block placeHolder;

	handle_ = &handle;

	// version
	if (!readBuffer(placeHolder, 1)) { return false; }
	bool version4 = placeHolder.toUnsignedInt() == 4;
	ove_->setIsVersion4(version4);

	QString str = QString("This file is created by Overture ") + (version4 ? "4" : "3");
	messageOut(str);

	if( !jump(6) ) { return false; }

	// show page margin
	if (!readBuffer(placeHolder, 1)) { return false; }
	ove_->setShowPageMargin(placeHolder.toBoolean());

	if( !jump(1) ) { return false; }

	// transpose track
	if (!readBuffer(placeHolder, 1)) { return false; }
	ove_->setShowTransposeTrack(placeHolder.toBoolean());

	// play repeat
	if (!readBuffer(placeHolder, 1)) { return false; }
	ove_->setPlayRepeat(placeHolder.toBoolean());

	// play style
	if (!readBuffer(placeHolder, 1)) { return false; }
	OveSong::PlayStyle style = OveSong::Record;
	if(placeHolder.toUnsignedInt() == 1){
		style = OveSong::Swing;
	}
	else if(placeHolder.toUnsignedInt() == 2){
		style = OveSong::Notation;
	}
	ove_->setPlayStyle(style);

	// show line break
	if (!readBuffer(placeHolder, 1)) { return false; }
	ove_->setShowLineBreak(placeHolder.toBoolean());

	// show ruler
	if (!readBuffer(placeHolder, 1)) { return false; }
	ove_->setShowRuler(placeHolder.toBoolean());

	// show color
	if (!readBuffer(placeHolder, 1)) { return false; }
	ove_->setShowColor(placeHolder.toBoolean());

	return true;
}

///////////////////////////////////////////////////////////////////////////////
TrackParse::TrackParse(OveSong* ove)
:BasicParse(ove) {
}

TrackParse::~TrackParse() {
}

void TrackParse::setTrack(SizeChunk* chunk) {
	chunk_ = chunk;
}

bool TrackParse::parse() {
	Block* dataBlock = chunk_->getDataBlock();
	unsigned int blockSize = ove_->getIsVersion4() ? chunk_->getSizeBlock()->toSize() : SizeChunk::version3TrackSize;
	StreamHandle handle(dataBlock->data(), blockSize);
	Block placeHolder;

	handle_ = &handle;

	Track* oveTrack = new Track();
	ove_->addTrack(oveTrack);

	// 2 32bytes long track name buffer
	if( !readBuffer(placeHolder, 32) ) { return false; }
	oveTrack->setName(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

	if( !readBuffer(placeHolder, 32) ) { return false; }
	oveTrack->setBriefName(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

	if( !jump(8) ) { return false; } //0x fffa0012 fffa0012
	if( !jump(1) ) { return false; }

	// patch
	if( !readBuffer(placeHolder, 1) ) { return false; }
	unsigned int thisByte = placeHolder.toInt();
	oveTrack->setPatch(thisByte&0x7f);

	// show name
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setShowName(placeHolder.toBoolean());

	// show brief name
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setShowBriefName(placeHolder.toBoolean());

	if( !jump(1) ) { return false; }

	// show transpose
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setShowTranspose(placeHolder.toBoolean());

	if( !jump(1) ) { return false; }

	// mute
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setMute(placeHolder.toBoolean());

	// solo
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setSolo(placeHolder.toBoolean());

	if( !jump(1) ) { return false; }

	// show key each line
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setShowKeyEachLine(placeHolder.toBoolean());

	// voice count
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setVoiceCount(placeHolder.toUnsignedInt());

	if( !jump(3) ) { return false; }

	// transpose value [-127, 127]
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setTranspose(placeHolder.toInt());

	if( !jump(2) ) { return false; }

	// start clef
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setStartClef(placeHolder.toUnsignedInt());

	// transpose celf
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setTransposeClef(placeHolder.toUnsignedInt());

	// start key
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setStartKey(placeHolder.toUnsignedInt());

	// display percent
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setDisplayPercent(placeHolder.toUnsignedInt());

	// show leger line
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setShowLegerLine(placeHolder.toBoolean());

	// show clef
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setShowClef(placeHolder.toBoolean());

	// show time signature
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setShowTimeSignature(placeHolder.toBoolean());

	// show key signature
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setShowKeySignature(placeHolder.toBoolean());

	// show barline
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setShowBarline(placeHolder.toBoolean());

	// fill with rest
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setFillWithRest(placeHolder.toBoolean());

	// flat tail
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setFlatTail(placeHolder.toBoolean());

	// show clef each line
	if( !readBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setShowClefEachLine(placeHolder.toBoolean());

	if( !jump(12) ) { return false; }

	// 8 voices
	unsigned int i;
	QList<Voice*> voices;
	for( i=0; i<8; ++i ) {
		Voice* voicePtr = new Voice();

		if( !jump(5) ) { return false; }

		// channel
		if( !readBuffer(placeHolder, 1) ) { return false; }
		voicePtr->setChannel(placeHolder.toUnsignedInt());

		// volume
		if( !readBuffer(placeHolder, 1) ) { return false; }
		voicePtr->setVolume(placeHolder.toInt());

		// pitch shift
		if( !readBuffer(placeHolder, 1) ) { return false; }
		voicePtr->setPitchShift(placeHolder.toInt());

		// pan
		if( !readBuffer(placeHolder, 1) ) { return false; }
		voicePtr->setPan(placeHolder.toInt());

		if( !jump(6) ) { return false; }

		// patch
		if( !readBuffer(placeHolder, 1) ) { return false; }
		voicePtr->setPatch(placeHolder.toInt());

		voices.push_back(voicePtr);
	}

	// stem type
	for( i=0; i<8; ++i ) {
		if( !readBuffer(placeHolder, 1) ) { return false; }
		voices[i]->setStemType(placeHolder.toUnsignedInt());

		oveTrack->addVoice(voices[i]);
	}

	// percussion define
	QList<Track::DrumNode> nodes;
	for(i=0; i<16; ++i) {
		nodes.push_back(Track::DrumNode());
	}

	// line
	for( i=0; i<16; ++i ) {
		if( !readBuffer(placeHolder, 1) ) { return false; }
		nodes[i].line_ = placeHolder.toInt();
	}

	// head type
	for( i=0; i<16; ++i ) {
		if( !readBuffer(placeHolder, 1) ) { return false; }
		nodes[i].headType_ = placeHolder.toUnsignedInt();
	}

	// pitch
	for( i=0; i<16; ++i ) {
		if( !readBuffer(placeHolder, 1) ) { return false; }
		nodes[i].pitch_ = placeHolder.toUnsignedInt();
	}

	// voice
	for( i=0; i<16; ++i ) {
		if( !readBuffer(placeHolder, 1) ) { return false; }
		nodes[i].voice_ = placeHolder.toUnsignedInt();
	}

	for( i=0; i<nodes.size(); ++i ) {
		oveTrack->addDrum(nodes[i]);
	}

/*	if( !Jump(17) ) { return false; }

	// voice 0 channel
	if( !ReadBuffer(placeHolder, 1) ) { return false; }
	oveTrack->setChannel(placeHolder.toUnsignedInt());

	// to be continued. if anything important...*/

	return true;
}

///////////////////////////////////////////////////////////////////////////////
GroupParse::GroupParse(OveSong* ove)
:BasicParse(ove) {
}

GroupParse::~GroupParse(){
	sizeChunks_.clear();
}

void GroupParse::addSizeChunk(SizeChunk* sizeChunk) {
	sizeChunks_.push_back(sizeChunk);
}

bool GroupParse::parse() {
	return false;
}

///////////////////////////////////////////////////////////////////////////////
PageGroupParse::PageGroupParse(OveSong* ove)
:BasicParse(ove) {
}

PageGroupParse::~PageGroupParse(){
	pageChunks_.clear();
}

void PageGroupParse::addPage(SizeChunk* chunk) {
	pageChunks_.push_back(chunk);
}

bool PageGroupParse::parse() {
	if( pageChunks_.empty() ) { return false; }

	unsigned int i;
	for( i=0; i<pageChunks_.size(); ++i ) {
		Page* page = new Page();
		ove_->addPage(page);

		if( !parsePage(pageChunks_[i], page) ) { return false; }
	}

	return true;
}

bool PageGroupParse::parsePage(SizeChunk* chunk, Page* page) {
	Block placeHolder(2);
	StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

	handle_ = &handle;

	// begin line
	if( !readBuffer(placeHolder, 2) ) { return false; }
	page->setBeginLine(placeHolder.toUnsignedInt());

	// line count
	if( !readBuffer(placeHolder, 2) ) { return false; }
	page->setLineCount(placeHolder.toUnsignedInt());

	if( !jump(4) ) { return false; }

	// staff interval
	if( !readBuffer(placeHolder, 2) ) { return false; }
	page->setStaffInterval(placeHolder.toUnsignedInt());

	// line interval
	if( !readBuffer(placeHolder, 2) ) { return false; }
	page->setLineInterval(placeHolder.toUnsignedInt());

	// staff inline interval
	if( !readBuffer(placeHolder, 2) ) { return false; }
	page->setStaffInlineInterval(placeHolder.toUnsignedInt());

	// line bar count
	if( !readBuffer(placeHolder, 2) ) { return false; }
	page->setLineBarCount(placeHolder.toUnsignedInt());

	// page line count
	if( !readBuffer(placeHolder, 2) ) { return false; }
	page->setPageLineCount(placeHolder.toUnsignedInt());

	// left margin
	if( !readBuffer(placeHolder, 4) ) { return false; }
	page->setLeftMargin(placeHolder.toUnsignedInt());

	// top margin
	if( !readBuffer(placeHolder, 4) ) { return false; }
	page->setTopMargin(placeHolder.toUnsignedInt());

	// right margin
	if( !readBuffer(placeHolder, 4) ) { return false; }
	page->setRightMargin(placeHolder.toUnsignedInt());

	// bottom margin
	if( !readBuffer(placeHolder, 4) ) { return false; }
	page->setBottomMargin(placeHolder.toUnsignedInt());

	// page width
	if( !readBuffer(placeHolder, 4) ) { return false; }
	page->setPageWidth(placeHolder.toUnsignedInt());

	// page height
	if( !readBuffer(placeHolder, 4) ) { return false; }
	page->setPageHeight(placeHolder.toUnsignedInt());

	handle_ = NULL;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
StaffCountGetter::StaffCountGetter(OveSong* ove)
:BasicParse(ove) {
}

unsigned int StaffCountGetter::getStaffCount(SizeChunk* chunk) {
	StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());
	Block placeHolder;

	handle_ = &handle;

	if( !jump(6) ) { return false; }

	// staff count
	if( !readBuffer(placeHolder, 2) ) { return false; }
	return placeHolder.toUnsignedInt();
}

///////////////////////////////////////////////////////////////////////////////
LineGroupParse::LineGroupParse(OveSong* ove) :
	BasicParse(ove), chunk_(NULL) {
}

LineGroupParse::~LineGroupParse(){
	chunk_ = NULL;
	lineChunks_.clear();
	staffChunks_.clear();
}

void LineGroupParse::setLineGroup(GroupChunk* chunk) {
	chunk_ = chunk;
}

void LineGroupParse::addLine(SizeChunk* chunk) {
	lineChunks_.push_back(chunk);
}

void LineGroupParse::addStaff(SizeChunk* chunk) {
	staffChunks_.push_back(chunk);
}

bool LineGroupParse::parse() {
	if( lineChunks_.empty() || staffChunks_.size() % lineChunks_.size() != 0 ) { return false; }

	unsigned int i;
	unsigned int j;
	unsigned int lineStaffCount = staffChunks_.size() / lineChunks_.size();

	for( i=0; i<lineChunks_.size(); ++i ) {
		Line* linePtr = new Line();

		ove_->addLine(linePtr);

		if( !parseLine(lineChunks_[i], linePtr) ) { return false; }

		for( j=lineStaffCount*i; j<lineStaffCount*(i+1); ++j ) {
			Staff* staffPtr = new Staff();

			linePtr->addStaff(staffPtr);

			if( !parseStaff(staffChunks_[j], staffPtr) ) { return false; }
		}
	}

	return true;
}

bool LineGroupParse::parseLine(SizeChunk* chunk, Line* line) {
	Block placeHolder;

	StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

	handle_ = &handle;

	if( !jump(2) ) { return false; }

	// begin bar
	if( !readBuffer(placeHolder, 2) ) { return false; }
	line->setBeginBar(placeHolder.toUnsignedInt());

	// bar count
	if( !readBuffer(placeHolder, 2) ) { return false; }
	line->setBarCount(placeHolder.toUnsignedInt());

	if( !jump(6) ) { return false; }

	// y offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	line->setYOffset(placeHolder.toInt());

	// left x offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	line->setLeftXOffset(placeHolder.toInt());

	// right x offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	line->setRightXOffset(placeHolder.toInt());

	if( !jump(4) ) { return false; }

	handle_ = NULL;

	return true;
}

bool LineGroupParse::parseStaff(SizeChunk* chunk, Staff* staff) {
	Block placeHolder;

	StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

	handle_ = &handle;

	if( !jump(7) ) { return false; }

	// clef
	if( !readBuffer(placeHolder, 1) ) { return false; }
	staff->setClefType(placeHolder.toUnsignedInt());

	// key
	if( !readBuffer(placeHolder, 1) ) { return false; }
	staff->setKeyType(oveKeyToKey(placeHolder.toUnsignedInt()));

	if( !jump(2) ) { return false; }

	// visible
	if( !readBuffer(placeHolder, 1) ) { return false; }
	staff->setVisible(placeHolder.toBoolean());

	if( !jump(12) ) { return false; }

	// y offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	staff->setYOffset(placeHolder.toInt());

	int jumpAmount = ove_->getIsVersion4() ? 26 : 18;
	if( !jump(jumpAmount) ) { return false; }

	// group type
	if( !readBuffer(placeHolder, 1) ) { return false; }
	GroupType groupType = Group_None;
	if(placeHolder.toUnsignedInt() == 1) {
		groupType = Group_Brace;
	} else if(placeHolder.toUnsignedInt() == 2) {
		groupType = Group_Bracket;
	}
	staff->setGroupType(groupType);

	// group staff count
	if( !readBuffer(placeHolder, 1) ) { return false; }
	staff->setGroupStaffCount(placeHolder.toUnsignedInt());

	handle_ = NULL;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
BarsParse::BarsParse(OveSong* ove) :
	BasicParse(ove) {
}

BarsParse::~BarsParse(){
	measureChunks_.clear();
	conductChunks_.clear();
	bdatChunks_.clear();
}

void BarsParse::addMeasure(SizeChunk* chunk) {
	measureChunks_.push_back(chunk);
}

void BarsParse::addConduct(SizeChunk* chunk) {
	conductChunks_.push_back(chunk);
}

void BarsParse::addBdat(SizeChunk* chunk) {
	bdatChunks_.push_back(chunk);
}

bool BarsParse::parse() {
	int i;
	int trackMeasureCount = ove_->getTrackBarCount();
	int trackCount = ove_->getTrackCount();
	int measureDataCount = trackCount * measureChunks_.size();
	QList<Measure*> measures;
	QList<MeasureData*> measureDatas;

	if( measureChunks_.empty() ||
		measureChunks_.size() != conductChunks_.size() ||
		(int)bdatChunks_.size() != measureDataCount ) {
		return false;
	}

	// add to ove
	for ( i=0; i<(int)measureChunks_.size(); ++i ) {
		Measure* measure = new Measure(i);

		measures.push_back(measure);
		ove_->addMeasure(measure);
	}

	for ( i=0; i<measureDataCount; ++i ) {
		MeasureData* oveMeasureData = new MeasureData();

		measureDatas.push_back(oveMeasureData);
		ove_->addMeasureData(oveMeasureData);
	}

	for( i=0; i<(int)measureChunks_.size(); ++i ) {
		Measure* measure = measures[i];

		// MEAS
		if( !parseMeas(measure, measureChunks_[i]) ) {
			QString ss = "failed in parse MEAS " + i;
			messageOut(ss);

			return false;
		}
	}

	for( i=0; i<(int)conductChunks_.size(); ++i ) {
		// COND
		if( !parseCond(measures[i], measureDatas[i], conductChunks_[i]) ) {
			QString ss = "failed in parse COND " + i;
			messageOut(ss);

			return false;
		}
	}

	for( i=0; i<(int)bdatChunks_.size(); ++i ) {
		int measId = i % trackMeasureCount;

		// BDAT
		if( !parseBdat(measures[measId], measureDatas[i], bdatChunks_[i]) ) {
			QString ss = "failed in parse BDAT " + i;
			messageOut(ss);

			return false;
		}

		if( notify_ != NULL ) {
			int measureID = i % trackMeasureCount;
			int trackID = i / trackMeasureCount;

			//msg.msg_ = OVE_IMPORT_POS;
			//msg.param1_ = (measureID<<16) + trackMeasureCount;
			//msg.param2_ = (trackID<<16) + trackCount;

			notify_->loadPosition(measureID, trackMeasureCount, trackID, trackCount);
		}
	}

	return true;
}

bool BarsParse::parseMeas(Measure* measure, SizeChunk* chunk) {
	Block placeHolder;

	StreamHandle measureHandle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

	handle_ = &measureHandle;

	if( !jump(2) ) { return false; }

	// multi-measure rest
	if( !readBuffer(placeHolder, 1) ) { return false; }
	measure->setIsMultiMeasureRest(placeHolder.toBoolean());

	// pickup
	if( !readBuffer(placeHolder, 1) ) { return false; }
	measure->setIsPickup(placeHolder.toBoolean());

	if( !jump(4) ) { return false; }

	// left barline
	if( !readBuffer(placeHolder, 1) ) { return false; }
	measure->setLeftBarline(placeHolder.toUnsignedInt());

	// right barline
	if( !readBuffer(placeHolder, 1) ) { return false; }
	measure->setRightBarline(placeHolder.toUnsignedInt());

	// tempo
	if( !readBuffer(placeHolder, 2) ) { return false; }
	double tempo = ((double)placeHolder.toUnsignedInt());
	if( ove_->getIsVersion4() ) {
		tempo /= 100.0;
	}
	measure->setTypeTempo(tempo);

	// bar length(tick)
	if( !readBuffer(placeHolder, 2) ) { return false; }
	measure->setLength(placeHolder.toUnsignedInt());

	if( !jump(6) ) { return false; }

	// bar number offset
	if( !parseOffsetElement(measure->getBarNumber()) ) { return false; }

	if( !jump(2) ) { return false; }

	// multi-measure rest count
	if( !readBuffer(placeHolder, 2) ) { return false; }
	measure->setMultiMeasureRestCount(placeHolder.toUnsignedInt());

	handle_ = NULL;

	return true;
}

bool BarsParse::parseCond(Measure* measure, MeasureData* measureData, SizeChunk* chunk) {
	Block placeHolder;

	StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

	handle_ = &handle;

	// item count
	if( !readBuffer(placeHolder, 2) ) { return false; }
	unsigned int cnt = placeHolder.toUnsignedInt();

	if( !parseTimeSignature(measure, 36) ) { return false; }

	for( unsigned int i=0; i<cnt; ++i ) {
		if( !readBuffer(placeHolder, 2) ) { return false; }
		unsigned int twoByte = placeHolder.toUnsignedInt();
		unsigned int oldBlockSize = twoByte - 11;
		unsigned int newBlockSize = twoByte - 7;

		// type id
		if( !readBuffer(placeHolder, 1) ) { return false; }
		unsigned int thisByte = placeHolder.toUnsignedInt();
		CondType type;

		if( !getCondElementType(thisByte, type) ) { return false; }

		switch (type) {
		case Cond_Bar_Number: {
			if (!parseBarNumber(measure, twoByte - 1)) {
				return false;
			}
			break;
		}
		case Cond_Repeat: {
			if (!parseRepeatSymbol(measureData, oldBlockSize)) {
				return false;
			}
			break;
		}
		case Cond_Numeric_Ending: {
			if (!parseNumericEndings(measureData, oldBlockSize)) {
				return false;
			}
			break;
		}
		case Cond_Decorator: {
			if (!parseDecorators(measureData, newBlockSize)) {
				return false;
			}
			break;
		}
		case Cond_Tempo: {
			if (!parseTempo(measureData, newBlockSize)) {
				return false;
			}
			break;
		}
		case Cond_Text: {
			if (!parseText(measureData, newBlockSize)) {
				return false;
			}
			break;
		}
		case Cond_Expression: {
			if (!parseExpressions(measureData, newBlockSize)) {
				return false;
			}
			break;
		}
		case Cond_Time_Parameters: {
			if (!parseTimeSignatureParameters(measure, newBlockSize)) {
				return false;
			}
			break;
		}
		case Cond_Barline_Parameters: {
			if (!parseBarlineParameters(measure, newBlockSize)) {
				return false;
			}
			break;
		}
		default: {
			if (!jump(newBlockSize)) {
				return false;
			}
			break;
		}
		}
	}

	handle_ = NULL;

	return true;
}

bool BarsParse::parseTimeSignature(Measure* measure, int /*length*/) {
	Block placeHolder;

	TimeSignature* timeSignature = measure->getTime();

	// numerator
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setNumerator(placeHolder.toUnsignedInt());

	// denominator
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setDenominator(placeHolder.toUnsignedInt());

	if( !jump(2) ) { return false; }

	// beat length
	if( !readBuffer(placeHolder, 2) ) { return false; }
	timeSignature->setBeatLength(placeHolder.toUnsignedInt());

	// bar length
	if( !readBuffer(placeHolder, 2) ) { return false; }
	timeSignature->setBarLength(placeHolder.toUnsignedInt());

	if( !jump(4) ) { return false; }

	// is symbol
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setIsSymbol(placeHolder.toBoolean());

	if( !jump(1) ) { return false; }

	// replace font
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setReplaceFont(placeHolder.toBoolean());

	// color
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setColor(placeHolder.toUnsignedInt());

	// show
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setShow(placeHolder.toBoolean());

	// show beat group
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setShowBeatGroup(placeHolder.toBoolean());

	if( !jump(6) ) { return false; }

	// numerator 1, 2, 3
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setGroupNumerator1(placeHolder.toUnsignedInt());
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setGroupNumerator2(placeHolder.toUnsignedInt());
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setGroupNumerator3(placeHolder.toUnsignedInt());

	// denominator
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setGroupDenominator1(placeHolder.toUnsignedInt());
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setGroupDenominator2(placeHolder.toUnsignedInt());
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setGroupDenominator3(placeHolder.toUnsignedInt());

	// beam group 1~4
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setBeamGroup1(placeHolder.toUnsignedInt());
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setBeamGroup2(placeHolder.toUnsignedInt());
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setBeamGroup3(placeHolder.toUnsignedInt());
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->setBeamGroup4(placeHolder.toUnsignedInt());

	// beam 16th
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->set16thBeamCount(placeHolder.toUnsignedInt());

	// beam 32th
	if( !readBuffer(placeHolder, 1) ) { return false; }
	timeSignature->set32thBeamCount(placeHolder.toUnsignedInt());

	return true;
}

bool BarsParse::parseTimeSignatureParameters(Measure* measure, int length) {
	Block placeHolder;
	TimeSignature* ts = measure->getTime();

	int cursor = ove_->getIsVersion4() ? 10 : 8;
	if( !jump(cursor) ) { return false; }

	// numerator
	if( !readBuffer(placeHolder, 1) ) { return false; }
	unsigned int numerator = placeHolder.toUnsignedInt();

	cursor = ove_->getIsVersion4() ? 11 : 9;
	if( ( length - cursor ) % 8 != 0 || (length - cursor) / 8 != (int)numerator ) {
		return false;
	}

	for( unsigned int i =0; i<numerator; ++i ) {
		// beat start unit
		if( !readBuffer(placeHolder, 2) ) { return false; }
		int beatStart = placeHolder.toUnsignedInt();

		// beat length unit
		if( !readBuffer(placeHolder, 2) ) { return false; }
		int beatLength = placeHolder.toUnsignedInt();

		if( !jump(2) ) { return false; }

		// beat start tick
		if( !readBuffer(placeHolder, 2) ) { return false; }
		int beatStartTick = placeHolder.toUnsignedInt();

		ts->addBeat(beatStart, beatLength, beatStartTick);
	}

	ts->endAddBeat();

	return true;
}

bool BarsParse::parseBarlineParameters(Measure* measure, int /*length*/) {
	Block placeHolder;

	int cursor = ove_->getIsVersion4() ? 12 : 10;
	if( !jump(cursor) ) { return false; }

	// repeat count
	if( !readBuffer(placeHolder, 1) ) { return false; }
	int repeatCount = placeHolder.toUnsignedInt();

	measure->setBackwardRepeatCount(repeatCount);

	if( !jump(6) ) { return false; }

	return true;
}

bool BarsParse::parseNumericEndings(MeasureData* measureData, int /*length*/) {
	Block placeHolder;

	NumericEnding* numeric = new NumericEnding();
	measureData->addCrossMeasureElement(numeric, true);

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(numeric) ) { return false; }

	if( !jump(6) ) { return false; }

	// measure count
	if( !readBuffer(placeHolder, 2) ) { return false; }
	//int offsetMeasure = placeHolder.toUnsignedInt() - 1;
	int offsetMeasure = placeHolder.toUnsignedInt();
	numeric->stop()->setMeasure(offsetMeasure);

	if( !jump(2) ) { return false; }

	// left x offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	numeric->getLeftShoulder()->setXOffset(placeHolder.toInt());

	// height
	if( !readBuffer(placeHolder, 2) ) { return false; }
	numeric->setHeight(placeHolder.toUnsignedInt());

	// left x offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	numeric->getRightShoulder()->setXOffset(placeHolder.toInt());

	if( !jump(2) ) { return false; }

	// y offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	numeric->getLeftShoulder()->setYOffset(placeHolder.toInt());
	numeric->getRightShoulder()->setYOffset(placeHolder.toInt());

	// number offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	numeric->getNumericHandle()->setXOffset(placeHolder.toInt());
	if( !readBuffer(placeHolder, 2) ) { return false; }
	numeric->getNumericHandle()->setYOffset(placeHolder.toInt());

	if( !jump(6) ) { return false; }

	// text size
	if( !readBuffer(placeHolder, 1) ) { return false; }
	unsigned int size = placeHolder.toUnsignedInt();

	// text : size maybe a huge value
	if( !readBuffer(placeHolder, size) ) { return false; }
	numeric->setText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

	// fix for wedding march.ove
	if( size % 2 == 0 ) {
		if( !jump(1) ) { return false; }
	}

	return true;
}

bool BarsParse::parseTempo(MeasureData* measureData, int /*length*/) {
	Block placeHolder;
	unsigned int thisByte;

	Tempo* tempo = new Tempo();
	measureData->addMusicData(tempo);

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(tempo) ) { return false; }

	if( !readBuffer(placeHolder, 1) ) { return false; }
	thisByte = placeHolder.toUnsignedInt();

	// show tempo
	tempo->setShowMark( (getHighNibble(thisByte) & 0x4) == 0x4 );
	// show before text
	tempo->setShowBeforeText( (getHighNibble(thisByte) & 0x8 ) == 0x8 ) ;
	// show parenthesis
	tempo->setShowParenthesis( (getHighNibble(thisByte) & 0x1 ) == 0x1 );
	// left note type
	tempo->setLeftNoteType( getLowNibble(thisByte) );

	if( !jump(1) ) { return false; }

	if( ove_->getIsVersion4() ) {
		if( !jump(2) ) { return false; }

		// tempo
		if( !readBuffer(placeHolder, 2) ) { return false; }
		tempo->setTypeTempo(placeHolder.toUnsignedInt()/100);
	} else {
		// tempo
		if( !readBuffer(placeHolder, 2) ) { return false; }
		tempo->setTypeTempo(placeHolder.toUnsignedInt());

		if( !jump(2) ) { return false; }
	}

	// offset
	if( !parseOffsetElement(tempo) ) { return false; }

	if( !jump(16) ) { return false; }

	// 31 bytes left text
	if( !readBuffer(placeHolder, 31) ) { return false; }
	tempo->setLeftText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

	if( !readBuffer(placeHolder, 1) ) { return false; }
	thisByte = placeHolder.toUnsignedInt();

	// swing eighth
	tempo->setSwingEighth(getHighNibble(thisByte)!=8);

	// right note type
	tempo->setRightNoteType(getLowNibble(thisByte));

	// right text
	if( ove_->getIsVersion4() ) {
		if( !readBuffer(placeHolder, 31) ) { return false; }
		tempo->setRightText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

		if( !jump(1) ) { return false; }
	}

	return true;
}

bool BarsParse::parseBarNumber(Measure* measure, int /*length*/) {
	Block placeHolder;

	BarNumber* barNumber = measure->getBarNumber();

	if( !jump(2) ) { return false; }

	// show on paragraph start
	if( !readBuffer(placeHolder, 1) ) { return false; }
	barNumber->setShowOnParagraphStart(getLowNibble(placeHolder.toUnsignedInt())==8);

	unsigned int blankSize = ove_->getIsVersion4() ? 9 : 7;
	if( !jump(blankSize) ) { return false; }

	// text align
	if( !readBuffer(placeHolder, 1) ) { return false; }
	barNumber->setAlign(placeHolder.toUnsignedInt());

	if( !jump(4) ) { return false; }

	// show flag
	if( !readBuffer(placeHolder, 1) ) { return false; }
	barNumber->setShowFlag(placeHolder.toUnsignedInt());

	if( !jump(10) ) { return false; }

	// bar range
	if( !readBuffer(placeHolder, 1) ) { return false; }
	barNumber->setShowEveryBarCount(placeHolder.toUnsignedInt());

	// prefix
	if( !readBuffer(placeHolder, 2) ) { return false; }
	barNumber->setPrefix(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

	if( !jump(18) ) { return false; }

	return true;
}

bool BarsParse::parseText(MeasureData* measureData, int length) {
	Block placeHolder;

	Text* text = new Text();
	measureData->addMusicData(text);

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(text) ) { return false; }

	// type
	if( !readBuffer(placeHolder, 1) ) { return false; }
	unsigned int thisByte = placeHolder.toUnsignedInt();
	bool includeLineBreak = ( (getHighNibble(thisByte)&0x2) != 0x2 );
	unsigned int id = getLowNibble(thisByte);
	Text::TextType textType = Text::Text_Rehearsal;

	if (id == 0) {
		textType = Text::Text_MeasureText;
	} else if (id == 1) {
		textType = Text::Text_SystemText;
	} else // id ==2
	{
		textType = Text::Text_Rehearsal;
	}

	text->setTextType(textType);

	if( !jump(1) ) { return false; }

	// x offset
	if( !readBuffer(placeHolder, 4) ) { return false; }
	text->setXOffset(placeHolder.toInt());

	// y offset
	if( !readBuffer(placeHolder, 4) ) { return false; }
	text->setYOffset(placeHolder.toInt());

	// width
	if( !readBuffer(placeHolder, 4) ) { return false; }
	text->setWidth(placeHolder.toUnsignedInt());

	// height
	if( !readBuffer(placeHolder, 4) ) { return false; }
	text->setHeight(placeHolder.toUnsignedInt());

	if( !jump(7) ) { return false; }

	// horizontal margin
	if( !readBuffer(placeHolder, 1) ) { return false; }
	text->setHorizontalMargin(placeHolder.toUnsignedInt());

	if( !jump(1) ) { return false; }

	// vertical margin
	if( !readBuffer(placeHolder, 1) ) { return false; }
	text->setVerticalMargin(placeHolder.toUnsignedInt());

	if( !jump(1) ) { return false; }

	// line thick
	if( !readBuffer(placeHolder, 1) ) { return false; }
	text->setLineThick(placeHolder.toUnsignedInt());

	if( !jump(2) ) { return false; }

	// text size
	if( !readBuffer(placeHolder, 2) ) { return false; }
	unsigned int size = placeHolder.toUnsignedInt();

	// text string, maybe huge
	if( !readBuffer(placeHolder, size) ) { return false; }
	text->setText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

	if( !includeLineBreak ) {
		if( !jump(6) ) { return false; }
	} else {
		unsigned int cursor = ove_->getIsVersion4() ? 43 : 41;
		cursor += size;

		// multi lines of text
		for( unsigned int i=0; i<2; ++i ) {
			if( (int)cursor < length ) {
				// line parameters count
				if( !readBuffer(placeHolder, 2) ) { return false; }
				unsigned int lineCount = placeHolder.toUnsignedInt();

				if( i==0 && int(cursor + 2 + 8*lineCount) > length ) {
					return false;
				}

				if( i==1 && int(cursor + 2 + 8*lineCount) != length ) {
					return false;
				}

				if( !jump(8*lineCount) ) { return false; }

				cursor += 2 + 8*lineCount;
			}
		}
	}

	return true;
}

bool BarsParse::parseRepeatSymbol(MeasureData* measureData, int /*length*/) {
	Block placeHolder;

	RepeatSymbol* repeat = new RepeatSymbol();
	measureData->addMusicData(repeat);

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(repeat) ) { return false; }

	// RepeatType
	if( !readBuffer(placeHolder, 1) ) { return false; }
	repeat->setRepeatType(placeHolder.toUnsignedInt());

	if( !jump(13) ) { return false; }

	// offset
	if( !parseOffsetElement(repeat) ) { return false; }

	if( !jump(15) ) { return false; }

	// size
	if( !readBuffer(placeHolder, 2) ) { return false; }
	unsigned int size = placeHolder.toUnsignedInt();

	// text, maybe huge
	if( !readBuffer(placeHolder, size) ) { return false; }
	repeat->setText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

	// last 0
	if( size % 2 == 0 ) {
		if( !jump(1) ) { return false; }
	}

	return true;
}

bool BarsParse::parseBdat(Measure* /*measure*/, MeasureData* measureData, SizeChunk* chunk) {
	Block placeHolder;
	StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

	handle_ = &handle;

	// parse here
	if( !readBuffer(placeHolder, 2) ) { return false; }
	unsigned int cnt = placeHolder.toUnsignedInt();

	for( unsigned int i=0; i<cnt; ++i ) {
		// 0x0028 or 0x0016 or 0x002C
		if( !readBuffer(placeHolder, 2) ) { return false; }
		unsigned int count = placeHolder.toUnsignedInt() - 7;

		// type id
		if( !readBuffer(placeHolder, 1) ) { return false; }
		unsigned int thisByte = placeHolder.toUnsignedInt();
		BdatType type;

		if( !getBdatElementType(thisByte, type) ) { return false; }

		switch( type ) {
		case Bdat_Raw_Note :
		case Bdat_Rest :
		case Bdat_Note : {
				if( !parseNoteRest(measureData, count, type) ) { return false; }
				break;
			}
		case Bdat_Beam : {
				if( !parseBeam(measureData, count) ) { return false; }
			    break;
			}
		case Bdat_Harmony : {
				if( !parseHarmony(measureData, count) ) { return false; }
				break;
			}
		case Bdat_Clef : {
				if( !parseClef(measureData, count) ) { return false; }
				break;
			}
		case Bdat_Dynamics : {
				if( !parseDynamics(measureData, count) ) { return false; }
			    break;
			}
		case Bdat_Wedge : {
				if( !parseWedge(measureData, count) ) { return false; }
			    break;
			}
		case Bdat_Glissando : {
				if( !parseGlissando(measureData, count) ) { return false; }
			    break;
			}
		case Bdat_Decorator : {
				if( !parseDecorators(measureData, count) ) { return false; }
			    break;
			}
		case Bdat_Key : {
				if( !parseKey(measureData, count) ) { return false; }
				break;
			}
		case Bdat_Lyric : {
				if( !parseLyric(measureData, count) ) { return false; }
			    break;
			}
		case Bdat_Octave_Shift: {
				if( !parseOctaveShift(measureData, count) ) { return false; }
			    break;
			}
		case Bdat_Slur : {
				if( !parseSlur(measureData, count) ) { return false; }
				break;
			}
		case Bdat_Text : {
				if( !parseText(measureData, count) ) { return false; }
				break;
			}
		case Bdat_Tie : {
				if( !parseTie(measureData, count) ) { return false; }
			    break;
			}
		case Bdat_Tuplet : {
				if( !parseTuplet(measureData, count) ) { return false; }
				break;
			}
		case Bdat_Guitar_Bend :
		case Bdat_Guitar_Barre : {
				if( !parseSizeBlock(count) ) { return false; }
				break;
			}
		case Bdat_Pedal: {
				if( !parsePedal(measureData, count) ) { return false; }
			    break;
			}
		case Bdat_KuoHao: {
				if( !parseKuohao(measureData, count) ) { return false; }
			    break;
			}
		case Bdat_Expressions: {
				if( !parseExpressions(measureData, count) ) { return false; }
			    break;
			}
		case Bdat_Harp_Pedal: {
				if( !parseHarpPedal(measureData, count) ) { return false; }
			    break;
			}
		case Bdat_Multi_Measure_Rest: {
				if( !parseMultiMeasureRest(measureData, count) ) { return false; }
				break;
			}
		case Bdat_Harmony_GuitarFrame: {
				if( !parseHarmonyGuitarFrame(measureData, count) ) { return false; }
				break;
			}
		case Bdat_Graphics_40:
		case Bdat_Graphics_RoundRect:
		case Bdat_Graphics_Rect:
		case Bdat_Graphics_Round:
		case Bdat_Graphics_Line:
		case Bdat_Graphics_Curve:
		case Bdat_Graphics_WedgeSymbol: {
				if( !parseSizeBlock(count) ) { return false; }
				break;
			}
		case Bdat_Midi_Controller : {
				if( !parseMidiController(measureData, count) ) { return false; }
			    break;
			}
		case Bdat_Midi_Program_Change : {
				if( !parseMidiProgramChange(measureData, count) ) { return false; }
			    break;
			}
		case Bdat_Midi_Channel_Pressure : {
				if( !parseMidiChannelPressure(measureData, count) ) { return false; }
			    break;
			}
		case Bdat_Midi_Pitch_Wheel : {
				if( !parseMidiPitchWheel(measureData, count) ) { return false; }
			    break;
			}
		default: {
				if( !jump(count) ) { return false; }
			    break;
			}
		}

		// if i==count-1 then is bar end place holder
	}

	handle_ = NULL;

	return true;
}

int getInt(int byte, int bits) {
	int num = 0;

	if( bits > 0 ) {
		int factor = int(pow(2.0, bits-1));
		num = (byte % (factor*2));

		if ( (byte & factor) == factor ) {
			num -= factor*2;
		}
	}

	return num;
}

bool BarsParse::parseNoteRest(MeasureData* measureData, int length, BdatType type) {
	NoteContainer* container = new NoteContainer();
	Block placeHolder;
	unsigned int thisByte;

	measureData->addNoteContainer(container);
	measureData->addMusicData(container);

	// note|rest & grace
	container->setIsRest(type==Bdat_Rest);
	container->setIsRaw(type==Bdat_Raw_Note);

	if( !readBuffer(placeHolder, 2) ) { return false; }
	thisByte = placeHolder.toUnsignedInt();
	container->setIsGrace( thisByte == 0x3C00 );
	container->setIsCue( thisByte == 0x4B40 || thisByte == 0x3240 );

	// show / hide
	if( !readBuffer(placeHolder, 1) ) { return false; }
	thisByte = placeHolder.toUnsignedInt();
	container->setShow(getLowNibble(thisByte)!=0x8);

	// voice
	container->setVoice(getLowNibble(thisByte)&0x7);

	// common
	if( !parseCommonBlock(container) ) { return false; }

	// tuplet
	if( !readBuffer(placeHolder, 1) ) { return false; }
	container->setTuplet(placeHolder.toUnsignedInt());

	// space
	if( !readBuffer(placeHolder, 1) ) { return false; }
	container->setSpace(placeHolder.toUnsignedInt());

	// in beam
	if( !readBuffer(placeHolder, 1) ) { return false; }
	thisByte = placeHolder.toUnsignedInt();
	bool inBeam = ( getHighNibble(thisByte) & 0x1 ) == 0x1;
	container->setInBeam(inBeam);

	// grace NoteType
	container->setGraceNoteType((NoteType)getHighNibble(thisByte));

	// dot
	container->setDot(getLowNibble(thisByte)&0x03);

	// NoteType
	if( !readBuffer(placeHolder, 1) ) { return false; }
	thisByte = placeHolder.toUnsignedInt();
	container->setNoteType((NoteType)getLowNibble(thisByte));

	int cursor = 0;

	if( type == Bdat_Rest ) {
		Note* restPtr = new Note();
		container->addNoteRest(restPtr);
		restPtr->setIsRest(true);

		// line
		if( !readBuffer(placeHolder, 1) ) { return false; }
		restPtr->setLine(placeHolder.toInt());

		if( !jump(1) ) { return false; }

		cursor = ove_->getIsVersion4() ? 16 : 14;
	} else // type == Bdat_Note || type == Bdat_Raw_Note
	{
		// stem up 0x80, stem down 0x00
		if( !readBuffer(placeHolder, 1) ) { return false; }
		thisByte = placeHolder.toUnsignedInt();
		container->setStemUp((getHighNibble(thisByte)&0x8)==0x8);

		// stem length
		int stemOffset = thisByte%0x80;
		container->setStemLength(getInt(stemOffset, 7)+7/*3.5 line span*/);

		// show stem 0x00, hide stem 0x40
		if( !readBuffer(placeHolder, 1) ) { return false; }
		bool hideStem = getHighNibble(thisByte)==0x4;
		container->setShowStem(!hideStem);

		if( !jump(1) ) { return false; }

		// note count
		if( !readBuffer(placeHolder, 1) ) { return false; }
		unsigned int noteCount = placeHolder.toUnsignedInt();
		unsigned int i;

		// each note 16 bytes
		for( i=0; i<noteCount; ++i ) {
			Note* notePtr = new Note();
			notePtr->setIsRest(false);

			container->addNoteRest(notePtr);

			// note show / hide
			if( !readBuffer(placeHolder, 1) ) { return false; }
			thisByte = placeHolder.toUnsignedInt();
			notePtr->setShow((thisByte&0x80) != 0x80);

			// note head type
			notePtr->setHeadType(thisByte&0x7f);

			// tie pos
			if( !readBuffer(placeHolder, 1) ) { return false; }
			thisByte = placeHolder.toUnsignedInt();
			notePtr->setTiePos(getHighNibble(thisByte));

			// offset staff, in {-1, 0, 1}
			if( !readBuffer(placeHolder, 1) ) { return false; }
			thisByte = getLowNibble(placeHolder.toUnsignedInt());
			int offsetStaff = 0;
			if( thisByte == 1 ) { offsetStaff = 1; }
			if( thisByte == 7 ) { offsetStaff = -1; }
			notePtr->setOffsetStaff(offsetStaff);

			// accidental
			if( !readBuffer(placeHolder, 1) ) { return false; }
			thisByte = placeHolder.toUnsignedInt();
			notePtr->setAccidental(getLowNibble(thisByte));
			// accidental 0: influenced by key, 4: influenced by previous accidental in measure
			bool notShow = ( getHighNibble(thisByte) == 0 ) || ( getHighNibble(thisByte) == 4 );
			notePtr->setShowAccidental(!notShow);

			if( !jump(1) ) { return false; }

			// line
			if( !readBuffer(placeHolder, 1) ) { return false; }
			notePtr->setLine(placeHolder.toInt());

			if( !jump(1) ) { return false; }

			// note
			if( !readBuffer(placeHolder, 1) ) { return false; }
			unsigned int note = placeHolder.toUnsignedInt();
			notePtr->setNote(note);

			// note on velocity
			if( !readBuffer(placeHolder, 1) ) { return false; }
			unsigned int onVelocity = placeHolder.toUnsignedInt();
			notePtr->setOnVelocity(onVelocity);

			// note off velocity
			if( !readBuffer(placeHolder, 1) ) { return false; }
			unsigned int offVelocity = placeHolder.toUnsignedInt();
			notePtr->setOffVelocity(offVelocity);

			if( !jump(2) ) { return false; }

			// length (tick)
			if( !readBuffer(placeHolder, 2) ) { return false; }
			container->setLength(placeHolder.toUnsignedInt());

			// offset tick
			if( !readBuffer(placeHolder, 2) ) { return false; }
			notePtr->setOffsetTick(placeHolder.toInt());
		}

		cursor = ove_->getIsVersion4() ? 18 : 16;
		cursor += noteCount * 16/*note size*/;
	}

	// articulation
	while ( cursor < length + 1/* 0x70 || 0x80 || 0x90 */ ) {
		Articulation* art = new Articulation();
		container->addArticulation(art);

		// block size
		if( !readBuffer(placeHolder, 2) ) { return false; }
		int blockSize = placeHolder.toUnsignedInt();

		// articulation type
		if( !readBuffer(placeHolder, 1) ) { return false; }
		art->setArtType(placeHolder.toUnsignedInt());

		// placement
		if( !readBuffer(placeHolder, 1) ) { return false; }
		art->setPlacementAbove(placeHolder.toUnsignedInt()!=0x00);	//0x00:below, 0x30:above

		// offset
		if( !parseOffsetElement(art) ) { return false; }

		if( !ove_->getIsVersion4() ) {
			if( blockSize - 8 > 0 ) {
				if( !jump(blockSize-8) ) { return false; }
			}
		} else {
			// setting
			if( !readBuffer(placeHolder, 1) ) { return false; }
			thisByte = placeHolder.toUnsignedInt();
			const bool changeSoundEffect = ( ( thisByte & 0x1 ) == 0x1 );
			const bool changeLength = ( ( thisByte & 0x2 ) == 0x2 );
			const bool changeVelocity = ( ( thisByte & 0x4 ) == 0x4 );
			//const bool changeExtraLength = ( ( thisByte & 0x20 ) == 0x20 );

			if( !jump(8) ) { return false; }

			// velocity type
			if( !readBuffer(placeHolder, 1) ) { return false; }
			thisByte = placeHolder.toUnsignedInt();
			if( changeVelocity ) {
				art->setVelocityType((Articulation::VelocityType)thisByte);
			}

			if( !jump(14) ) { return false; }

			// sound effect
			if( !readBuffer(placeHolder, 2) ) { return false; }
			int from = placeHolder.toInt();
			if( !readBuffer(placeHolder, 2) ) { return false; }
			int to = placeHolder.toInt();
			if( changeSoundEffect ) {
				art->setSoundEffect(from, to);
			}

			if( !jump(1) ) { return false; }

			// length percentage
			if( !readBuffer(placeHolder, 1) ) { return false; }
			if( changeLength ) {
				art->setLengthPercentage(placeHolder.toUnsignedInt());
			}

			// velocity
			if( !readBuffer(placeHolder, 2) ) { return false; }
			if( changeVelocity ) {
				art->setVelocityValue(placeHolder.toInt());
			}

			if( Articulation::isTrill(art->getArtType()) ) {
				if( !jump(8) ) { return false; }

				// trill note length
				if( !readBuffer(placeHolder, 1) ) { return false; }
				art->setTrillNoteLength(placeHolder.toUnsignedInt());

				// trill rate
				if( !readBuffer(placeHolder, 1) ) { return false; }
				thisByte = placeHolder.toUnsignedInt();
				NoteType trillNoteType = Note_Sixteen;
				switch ( getHighNibble(thisByte) ) {
				case 0:
					trillNoteType = Note_None;
					break;
				case 1:
					trillNoteType = Note_Sixteen;
					break;
				case 2:
					trillNoteType = Note_32;
					break;
				case 3:
					trillNoteType = Note_64;
					break;
				case 4:
					trillNoteType = Note_128;
					break;
				default:
					break;
				}
				art->setTrillRate(trillNoteType);

				// accelerate type
				art->setAccelerateType(thisByte&0xf);

				if( !jump(1) ) { return false; }

				// auxiliary first
				if( !readBuffer(placeHolder, 1) ) { return false; }
				art->setAuxiliaryFirst(placeHolder.toBoolean());

				if( !jump(1) ) { return false; }

				// trill interval
				if( !readBuffer(placeHolder, 1) ) { return false; }
				art->setTrillInterval(placeHolder.toUnsignedInt());
			} else {
				if( blockSize > 40 ) {
					if( !jump( blockSize - 40 ) ) { return false; }
				}
			}
		}

		cursor += blockSize;
	}

	return true;
}

int tupletToSpace(int tuplet) {
	int a(1);

	while( a*2 < tuplet ) {
		a *= 2;
	}

	return a;
}

bool BarsParse::parseBeam(MeasureData* measureData, int length) {
	unsigned int i;
	Block placeHolder;

	Beam* beam = new Beam();
	measureData->addCrossMeasureElement(beam, true);

	// maybe create tuplet, for < quarter & tool 3(
	bool createTuplet = false;
	int maxEndUnit = 0;
	Tuplet* tuplet = new Tuplet();

	// is grace
	if( !readBuffer(placeHolder, 1) ) { return false; }
	beam->setIsGrace(placeHolder.toBoolean());

	if( !jump(1) ) { return false; }

	// voice
	if( !readBuffer(placeHolder, 1) ) { return false; }
	beam->setVoice(getLowNibble(placeHolder.toUnsignedInt())&0x7);

	// common
	if( !parseCommonBlock(beam) ) { return false; }

	if( !jump(2) ) { return false; }

	// beam count
	if( !readBuffer(placeHolder, 1) ) { return false; }
	unsigned int beamCount = placeHolder.toUnsignedInt();

	if( !jump(1) ) { return false; }

	// left line
	if( !readBuffer(placeHolder, 1) ) { return false; }
	beam->getLeftLine()->setLine(placeHolder.toInt());

	// right line
	if( !readBuffer(placeHolder, 1) ) { return false; }
	beam->getRightLine()->setLine(placeHolder.toInt());

	if( ove_->getIsVersion4() ) {
		if( !jump(8) ) { return false; }
	}

	int currentCursor = ove_->getIsVersion4() ? 23 : 13;
	unsigned int count = (length - currentCursor)/16;

	if( count != beamCount ) { return false; }

	for( i=0; i<count; ++i ) {
		if( !jump(1) ) { return false; }

		// tuplet
		if( !readBuffer(placeHolder, 1) ) { return false; }
		int tupletCount = placeHolder.toUnsignedInt();
		if( tupletCount > 0 ) {
			createTuplet = true;
			tuplet->setTuplet(tupletCount);
			tuplet->setSpace(tupletToSpace(tupletCount));
		}

		// start / stop measure
		// line i start end position
		MeasurePos startMp;
		MeasurePos stopMp;

		if( !readBuffer(placeHolder, 1) ) { return false; }
		startMp.setMeasure(placeHolder.toUnsignedInt());
		if( !readBuffer(placeHolder, 1) ) { return false; }
		stopMp.setMeasure(placeHolder.toUnsignedInt());

		if( !readBuffer(placeHolder, 2) ) { return false; }
		startMp.setOffset(placeHolder.toInt());
		if( !readBuffer(placeHolder, 2) ) { return false; }
		stopMp.setOffset(placeHolder.toInt());

		beam->addLine(startMp, stopMp);

		if( stopMp.getOffset() > maxEndUnit ) {
			maxEndUnit = stopMp.getOffset();
		}

		if( i == 0 ) {
			if( !jump(4) ) { return false; }

			// left offset up+4, down-4
			if( !readBuffer(placeHolder, 2) ) { return false; }
			beam->getLeftShoulder()->setYOffset(placeHolder.toInt());

			// right offset up+4, down-4
			if( !readBuffer(placeHolder, 2) ) { return false; }
			beam->getRightShoulder()->setYOffset(placeHolder.toInt());
		} else {
			if( !jump(8) ) { return false; }
		}
	}

	const QList<QPair<MeasurePos, MeasurePos> > lines = beam->getLines();
	MeasurePos offsetMp;

	for( i=0; i<lines.size(); ++i ) {
		if( lines[i].second > offsetMp ) {
			offsetMp = lines[i].second;
		}
	}

	beam->stop()->setMeasure(offsetMp.getMeasure());
	beam->stop()->setOffset(offsetMp.getOffset());

	// a case that Tuplet block don't exist, and hide inside beam
	if( createTuplet ) {
		tuplet->copyCommonBlock(*beam);
		tuplet->getLeftLine()->setLine(beam->getLeftLine()->getLine());
		tuplet->getRightLine()->setLine(beam->getRightLine()->getLine());
		tuplet->stop()->setMeasure(beam->stop()->getMeasure());
		tuplet->stop()->setOffset(maxEndUnit);

		measureData->addCrossMeasureElement(tuplet, true);
	} else {
		delete tuplet;
	}

	return true;
}

bool BarsParse::parseTie(MeasureData* measureData, int /*length*/) {
	Block placeHolder;

	Tie* tie = new Tie();
	measureData->addCrossMeasureElement(tie, true);

	if( !jump(3) ) { return false; }

	// start common
	if( !parseCommonBlock(tie) ) { return false; }

	if( !jump(1) ) { return false; }

	// note
	if( !readBuffer(placeHolder, 1) ) { return false; }
	tie->setNote(placeHolder.toUnsignedInt());

	// pair lines
	if( !parsePairLinesBlock(tie) ) { return false; }

	// offset common
	if( !parseOffsetCommonBlock(tie) ) { return false; }

	// left shoulder offset
	if( !parseOffsetElement(tie->getLeftShoulder()) ) { return false; }

	// right shoulder offset
	if( !parseOffsetElement(tie->getRightShoulder()) ) { return false; }

	// height
	if( !readBuffer(placeHolder, 2) ) { return false; }
	tie->setHeight(placeHolder.toUnsignedInt());

	return true;
}

bool BarsParse::parseTuplet(MeasureData* measureData, int /*length*/) {
	Block placeHolder;

	Tuplet* tuplet = new Tuplet();
	measureData->addCrossMeasureElement(tuplet, true);

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(tuplet) ) { return false; }

	if( !jump(2) ) { return false; }

	// pair lines
	if( !parsePairLinesBlock(tuplet) ) { return false; }

	// offset common
	if( !parseOffsetCommonBlock(tuplet) ) { return false; }

	// left shoulder offset
	if( !parseOffsetElement(tuplet->getLeftShoulder()) ) { return false; }

	// right shoulder offset
	if( !parseOffsetElement(tuplet->getRightShoulder()) ) { return false; }

	if( !jump(2) ) { return false; }

	// height
	if( !readBuffer(placeHolder, 2) ) { return false; }
	tuplet->setHeight(placeHolder.toUnsignedInt());

	// tuplet
	if( !readBuffer(placeHolder, 1) ) { return false; }
	tuplet->setTuplet(placeHolder.toUnsignedInt());

	// space
	if( !readBuffer(placeHolder, 1) ) { return false; }
	tuplet->setSpace(placeHolder.toUnsignedInt());

	// mark offset
	if( !parseOffsetElement(tuplet->getMarkHandle()) ) { return false; }

	return true;
}

HarmonyType binaryToHarmonyType(int bin) {
	HarmonyType type = Harmony_maj;
	if( bin == 0x0091 ) {
		type = Harmony_maj;
	} else if( bin == 0x0089 ) {
		type = Harmony_min;
	} else if( bin == 0x0489 ) {
		type = Harmony_min7;
	} else if( bin == 0x0491 ) {
		type = Harmony_7;
	} else if( bin == 0x0495 ) {
		type = Harmony_9;
	} else if( bin == 0x0449 ) {
		type = Harmony_min7b5;
	} else if( bin == 0x04A1 ) {
		type = Harmony_7sus4;
	} else if( bin == 0x00A1 ) {
		type = Harmony_sus4;
	} else if( bin == 0x0049 ) {
		type = Harmony_dim;
	} else if( bin == 0x0249 ) {
		type = Harmony_dim7;
	} else if( bin == 0x0111 ) {
		type = Harmony_aug;
	} else if( bin == 0x0511 ) {
		type = Harmony_aug7;
	} else if( bin == 0x044D ) {
		type = Harmony_min9_b5;
	} else if( bin == 0x0499 ) {
		type = Harmony_7s9;
	} else if( bin == 0x0615 ) {
		type = Harmony_13;
	} else if( bin == 0x0289 ) {
		type = Harmony_min6;
	} else if( bin == 0x0291 ) {
		type = Harmony_6;
	} else if( bin == 0x0295 ) {
		type = Harmony_6; 	//6add9
	} else if( bin == 0x0095 ) {
		type = Harmony_min; //minor add9
	} else if( bin == 0x008D ) {
		type = Harmony_maj7;
	} else if( bin == 0x0891 ) {
		type = Harmony_maj7;
	} else if( bin == 0x0881 ) {
		type = Harmony_maj7_s5;	//maj7#5
	} else if( bin == 0x0911 ) {
		type = Harmony_maj7_s5;	//maj7#5
	} else if( bin == 0x0991 ) {
		type = Harmony_maj7_s11;//maj7#11
	} else if( bin == 0x0851 ) {
		type = Harmony_maj7_s11;//maj7#11
	} else if( bin == 0x08D1 ) {
		type = Harmony_maj9;
	} else if( bin == 0x0895 ) {
		type = Harmony_maj9_s5;	//maj9#5
	} else if( bin == 0x0995 ) {
		type = Harmony_maj13_s11;//maj9#11
	} else if( bin == 0x0855 ) {
		type = Harmony_maj9_s11;//maj9#11
	} else if( bin == 0x08D5 ) {
		type = Harmony_maj13;
	} else if( bin == 0x0A95 ) {
		type = Harmony_maj13_s11;//maj13#11
	} else if( bin == 0x0A55 ) {
		type = Harmony_maj13;	//maj13(no3)
	} else if( bin == 0x0A85 ) {
		type = Harmony_maj9_s5;	//maj13#5#11(no4)
	} else if( bin == 0x0B45 ) {
		type = Harmony_7b9;
	} else if( bin == 0x0493 ) {
		type = Harmony_7b5;
	} else if( bin == 0x0451 ) {
		type = Harmony_9b5;
	} else if( bin == 0x0455 ) {
		type = Harmony_7s9;		//7#5#9
	} else if( bin == 0x0519 ) {
		type = Harmony_7b9;		//7#5b9
	} else if( bin == 0x0513 ) {
		type = Harmony_aug7;	//aug9
	} else if( bin == 0x0515 ) {
		type = Harmony_sus4;	//sus9
	} else if( bin == 0x04A5 ) {
		type = Harmony_13b9;
	} else if( bin == 0x0613 ) {
		type = Harmony_13b9;	//13b9#11
	} else if( bin == 0x0611 ) {
		type = Harmony_13;
	} else if( bin == 0x0653 ) {
		type = Harmony_min;		//m(natural7)
	} else if( bin == 0x0889 ) {
		type = Harmony_min9;	//m9(natural7)
	} else if( bin == 0x088D ) {
		type = Harmony_min11;
	} else if( bin == 0x04AD ) {
		type = Harmony_9s11;
	} else if( bin == 0x04D5 ) {
		type = Harmony_7sus4;	//sus7
	} else if( bin == 0x0421 ) {
		type = Harmony_min11;
	} else if( bin == 0x04A9 ) {
		type = Harmony_min9;
	} else if( bin == 0x048D ) {
		type = Harmony_7b5b9;
	} else if( bin == 0x0453 ) {
		type = Harmony_maj;		//(no5)
	} else if( bin == 0x0011 ) {
		type = Harmony_maj7;	//(no3)
	} else if( bin == 0x0081 ) {
		type = Harmony_7;		//7(no3)
	} else if( bin == 0x0481 ) {
		type = Harmony_7;		//7(no5)
	} else if( bin == 0x0411 ) {
		type = Harmony_6;
	} else if( bin == 0x0291 ) {
		type = Harmony_sus4;	//sus(add9)
	} else if( bin == 0x00A5 ) {
		type = Harmony_13s9;	//13#9b5
	} else if( bin == 0x0659 ) {
		type = Harmony_sus4;	//sus(no5)
	} else if( bin == 0x0021 ) {
		type = Harmony_7b5b9;	//7b5b9#9
	} else if( bin == 0x045B ) {
		type = Harmony_13b5;	//13b5b9#9
	} else if( bin == 0x065B ) {
		type = Harmony_13b9;	//13b9#9
	} else if( bin == 0x061B ) {
		type = Harmony_7b9s9;	//7b9#9
	} else if( bin == 0x04B5 ) {
		type = Harmony_7;
	}

	return type;
}

bool BarsParse::parseHarmony(MeasureData* measureData, int /*length*/) {
	Block placeHolder;

	Harmony* harmony = new Harmony();
	measureData->addMusicData(harmony);

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(harmony) ) { return false; }

	// bass on bottom
	if( !readBuffer(placeHolder, 1) ) { return false; }
	harmony->setBassOnBottom((getHighNibble(placeHolder.toUnsignedInt())==0x4));

	if( !jump(1) ) { return false; }

	// y offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	harmony->setYOffset(placeHolder.toInt());

	// harmony type
	if( !readBuffer(placeHolder, 2) ) { return false; }
	harmony->setHarmonyType(binaryToHarmonyType(placeHolder.toUnsignedInt()));

	// root
	if( !readBuffer(placeHolder, 1) ) { return false; }
	harmony->setRoot(placeHolder.toInt());

	// bass
	if( !readBuffer(placeHolder, 1) ) { return false; }
	harmony->setBass(placeHolder.toInt());

	// angle
	if( !readBuffer(placeHolder, 2) ) { return false; }
	harmony->setAngle(placeHolder.toInt());

	if( ove_->getIsVersion4() ) {
		// length (tick)
		if( !readBuffer(placeHolder, 2) ) { return false; }
		harmony->setLength(placeHolder.toUnsignedInt());

		if( !jump(4) ) { return false; }
	}

	return true;
}

bool BarsParse::parseClef(MeasureData* measureData, int /*length*/) {
	Block placeHolder;

	Clef* clef = new Clef();
	measureData->addMusicData(clef);

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(clef) ) { return false; }

	// clef type
	if( !readBuffer(placeHolder, 1) ) { return false; }
	clef->setClefType(placeHolder.toUnsignedInt());

	// line
	if( !readBuffer(placeHolder, 1) ) { return false; }
	clef->setLine(placeHolder.toInt());

	if( !jump(2) ) { return false; }

	return true;
}

bool BarsParse::parseLyric(MeasureData* measureData, int length) {
	Block placeHolder;

	Lyric* lyric = new Lyric();
	measureData->addMusicData(lyric);

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(lyric) ) { return false; }

	if( !jump(2) ) { return false; }

	// offset
	if( !parseOffsetElement(lyric) ) { return false; }

	if( !jump(7) ) { return false; }

	// verse
	if( !readBuffer(placeHolder, 1) ) { return false; }
	lyric->setVerse(placeHolder.toUnsignedInt());

	if( ove_->getIsVersion4() ) {
		if( !jump(6) ) { return false; }

		// lyric
		if( length > 29 ) {
			if( !readBuffer(placeHolder, length-29) ) { return false; }
			lyric->setLyric(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));
		}
	}

	return true;
}

bool BarsParse::parseSlur(MeasureData* measureData, int /*length*/) {
	Block placeHolder;

	Slur* slur = new Slur();
	measureData->addCrossMeasureElement(slur, true);

	if( !jump(2) ) { return false; }

	// voice
	if( !readBuffer(placeHolder, 1) ) { return false; }
	slur->setVoice(getLowNibble(placeHolder.toUnsignedInt())&0x7);

	// common
	if( !parseCommonBlock(slur) ) { return false; }

	// show on top
	if( !readBuffer(placeHolder, 1) ) { return false; }
	slur->setShowOnTop(getHighNibble(placeHolder.toUnsignedInt())==0x8);

	if( !jump(1) ) { return false; }

	// pair lines
	if( !parsePairLinesBlock(slur) ) { return false; }

	// offset common
	if( !parseOffsetCommonBlock(slur) ) { return false; }

	// handle 1
	if( !parseOffsetElement(slur->getLeftShoulder()) ) { return false; }

	// handle 4
	if( !parseOffsetElement(slur->getRightShoulder()) ) { return false; }

	// handle 2
	if( !parseOffsetElement(slur->getHandle2()) ) { return false; }

	// handle 3
	if( !parseOffsetElement(slur->getHandle3()) ) { return false; }

	if( ove_->getIsVersion4() ) {
		if( !jump(3) ) { return false; }

		// note time percent
		if( !readBuffer(placeHolder, 1) ) { return false; }
		slur->setNoteTimePercent(placeHolder.toUnsignedInt());

		if( !jump(36) ) { return false; }
	}

	return true;
}

bool BarsParse::parseGlissando(MeasureData* measureData, int /*length*/) {
	Block placeHolder;

	Glissando* glissando = new Glissando();
	measureData->addCrossMeasureElement(glissando, true);

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(glissando) ) { return false; }

	// straight or wavy
	if( !readBuffer(placeHolder, 1) ) { return false; }
	unsigned int thisByte = placeHolder.toUnsignedInt();
	glissando->setStraightWavy(getHighNibble(thisByte)==4);

	if( !jump(1) ) { return false; }

	// pair lines
	if( !parsePairLinesBlock(glissando) ) { return false; }

	// offset common
	if( !parseOffsetCommonBlock(glissando) ) { return false; }

	// left shoulder
	if( !parseOffsetElement(glissando->getLeftShoulder()) ) { return false; }

	// right shoulder
	if( !parseOffsetElement(glissando->getRightShoulder()) ) { return false; }

	if( ove_->getIsVersion4() ) {
		if( !jump(1) ) { return false; }

		// line thick
		if( !readBuffer(placeHolder, 1) ) { return false; }
		glissando->setLineThick(placeHolder.toUnsignedInt());

		if( !jump(12) ) { return false; }

		// text 32 bytes
		if( !readBuffer(placeHolder, 32) ) { return false; }
		glissando->setText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

		if( !jump(6) ) { return false; }
	}

	return true;
}

bool getDecoratorType(
			unsigned int thisByte,
			bool& measureRepeat,
			Decorator::DecoratorType& decoratorType,
			bool& singleRepeat,
			ArticulationType& artType) {
	measureRepeat = false;
	decoratorType = Decorator::Decorator_Articulation;
	singleRepeat = true;
	artType = Articulation_None;

	switch (thisByte) {
	case 0x00: {
		decoratorType = Decorator::Decorator_Dotted_Barline;
		break;
	}
	case 0x30: {
		artType = Articulation_Open_String;
		break;
	}
	case 0x31: {
		artType = Articulation_Finger_1;
		break;
	}
	case 0x32: {
		artType = Articulation_Finger_2;
		break;
	}
	case 0x33: {
		artType = Articulation_Finger_3;
		break;
	}
	case 0x34: {
		artType = Articulation_Finger_4;
		break;
	}
	case 0x35: {
		artType = Articulation_Finger_5;
		break;
	}
	case 0x6B: {
		artType = Articulation_Flat_Accidental_For_Trill;
		break;
	}
	case 0x6C: {
		artType = Articulation_Sharp_Accidental_For_Trill;
		break;
	}
	case 0x6D: {
		artType = Articulation_Natural_Accidental_For_Trill;
		break;
	}
	case 0x8d: {
		measureRepeat = true;
		singleRepeat = true;
		break;
	}
	case 0x8e: {
		measureRepeat = true;
		singleRepeat = false;
		break;
	}
	case 0xA0: {
		artType = Articulation_Minor_Trill;
		break;
	}
	case 0xA1: {
		artType = Articulation_Major_Trill;
		break;
	}
	case 0xA2: {
		artType = Articulation_Trill_Section;
		break;
	}
	case 0xA6: {
		artType = Articulation_Turn;
		break;
	}
	case 0xA8: {
		artType = Articulation_Tremolo_Eighth;
		break;
	}
	case 0xA9: {
		artType = Articulation_Tremolo_Sixteenth;
		break;
	}
	case 0xAA: {
		artType = Articulation_Tremolo_Thirty_Second;
		break;
	}
	case 0xAB: {
		artType = Articulation_Tremolo_Sixty_Fourth;
		break;
	}
	case 0xB2: {
		artType = Articulation_Fermata;
		break;
	}
	case 0xB3: {
		artType = Articulation_Fermata_Inverted;
		break;
	}
	case 0xB9: {
		artType = Articulation_Pause;
		break;
	}
	case 0xBA: {
		artType = Articulation_Grand_Pause;
		break;
	}
	case 0xC0: {
		artType = Articulation_Marcato;
		break;
	}
	case 0xC1: {
		artType = Articulation_Marcato_Dot;
		break;
	}
	case 0xC2: {
		artType = Articulation_SForzando;
		break;
	}
	case 0xC3: {
		artType = Articulation_SForzando_Dot;
		break;
	}
	case 0xC4: {
		artType = Articulation_SForzando_Inverted;
		break;
	}
	case 0xC5: {
		artType = Articulation_SForzando_Dot_Inverted;
		break;
	}
	case 0xC6: {
		artType = Articulation_Staccatissimo;
		break;
	}
	case 0xC7: {
		artType = Articulation_Staccato;
		break;
	}
	case 0xC8: {
		artType = Articulation_Tenuto;
		break;
	}
	case 0xC9: {
		artType = Articulation_Natural_Harmonic;
		break;
	}
	case 0xCA: {
		artType = Articulation_Artificial_Harmonic;
		break;
	}
	case 0xCB: {
		artType = Articulation_Plus_Sign;
		break;
	}
	case 0xCC: {
		artType = Articulation_Up_Bow;
		break;
	}
	case 0xCD: {
		artType = Articulation_Down_Bow;
		break;
	}
	case 0xCE: {
		artType = Articulation_Up_Bow_Inverted;
		break;
	}
	case 0xCF: {
		artType = Articulation_Down_Bow_Inverted;
		break;
	}
	case 0xD0: {
		artType = Articulation_Pedal_Down;
		break;
	}
	case 0xD1: {
		artType = Articulation_Pedal_Up;
		break;
	}
	case 0xD6: {
		artType = Articulation_Heavy_Attack;
		break;
	}
	case 0xD7: {
		artType = Articulation_Heavier_Attack;
		break;
	}
	default:
		return false;
		break;
	}

	return true;
}

bool BarsParse::parseDecorators(MeasureData* measureData, int length) {
	Block placeHolder;
	MusicData* musicData = new MusicData();

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(musicData) ) { return false; }

	if( !jump(2) ) { return false; }

	// y offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	musicData->setYOffset(placeHolder.toInt());

	if( !jump(2) ) { return false; }

	// measure repeat | piano pedal | dotted barline | articulation
	if( !readBuffer(placeHolder, 1) ) { return false; }
	unsigned int thisByte = placeHolder.toUnsignedInt();

	Decorator::DecoratorType decoratorType;
	bool isMeasureRepeat;
	bool isSingleRepeat = true;
	ArticulationType artType = Articulation_None;

	getDecoratorType(thisByte, isMeasureRepeat, decoratorType, isSingleRepeat, artType);

	if( isMeasureRepeat ) {
		MeasureRepeat* measureRepeat = new MeasureRepeat();
		measureData->addCrossMeasureElement(measureRepeat, true);

		measureRepeat->copyCommonBlock(*musicData);
		measureRepeat->setYOffset(musicData->getYOffset());

		measureRepeat->setSingleRepeat(isSingleRepeat);
	} else {
		Decorator* decorator = new Decorator();
		measureData->addMusicData(decorator);

		decorator->copyCommonBlock(*musicData);
		decorator->setYOffset(musicData->getYOffset());

		decorator->setDecoratorType(decoratorType);
		decorator->setArticulationType(artType);
	}

	int cursor = ove_->getIsVersion4() ? 16 : 14;
	if( !jump(length-cursor) ) { return false; }

	return true;
}

bool BarsParse::parseWedge(MeasureData* measureData, int length) {
	Block placeHolder;
	Wedge* wedge = new Wedge();

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(wedge) ) { return false; }

	// wedge type
	if( !readBuffer(placeHolder, 1) ) { return false; }
	WedgeType wedgeType = Wedge_Cres_Line;
	bool wedgeOrExpression = true;
	unsigned int highHalfByte = getHighNibble(placeHolder.toUnsignedInt());
	unsigned int lowHalfByte = getLowNibble(placeHolder.toUnsignedInt());

	switch (highHalfByte) {
	case 0x0: {
		wedgeType = Wedge_Cres_Line;
		wedgeOrExpression = true;
		break;
	}
	case 0x4: {
		wedgeType = Wedge_Decresc_Line;
		wedgeOrExpression = true;
		break;
	}
	case 0x6: {
		wedgeType = Wedge_Decresc;
		wedgeOrExpression = false;
		break;
	}
	case 0x2: {
		wedgeType = Wedge_Cres;
		wedgeOrExpression = false;
		break;
	}
	default:
		break;
	}

	// 0xb | 0x8(ove3) , else 3, 0(ove3)
	if( (lowHalfByte & 0x8) == 0x8 ) {
		wedgeType = Wedge_Double_Line;
		wedgeOrExpression = true;
	}

	if( !jump(1) ) { return false; }

	// y offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	wedge->setYOffset(placeHolder.toInt());

	// wedge
	if( wedgeOrExpression ) {
		measureData->addCrossMeasureElement(wedge, true);
		wedge->setWedgeType(wedgeType);

		if( !jump(2) ) { return false; }

		// height
		if( !readBuffer(placeHolder, 2) ) { return false; }
		wedge->setHeight(placeHolder.toUnsignedInt());

		// offset common
		if( !parseOffsetCommonBlock(wedge) ) { return false; }

		int cursor = ove_->getIsVersion4() ? 21 : 19;
		if( !jump(length-cursor) ) { return false; }
	}
	// expression : cresc, decresc
	else {
		Expressions* express = new Expressions();
		measureData->addMusicData(express);

		express->copyCommonBlock(*wedge);
		express->setYOffset(wedge->getYOffset());

		if( !jump(4) ) { return false; }

		// offset common
		if( !parseOffsetCommonBlock(express) ) { return false; }

		if( ove_->getIsVersion4() ) {
			if( !jump(18) ) { return false; }

			// words
			if( length > 39 ) {
				if( !readBuffer(placeHolder, length-39) ) { return false; }
				express->setText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));
			}
		} else {
			QString str = wedgeType==Wedge_Cres ? "cresc" : "decresc";
			express->setText(str);

			if( !jump(8) ) { return false; }
		}
	}

	return true;
}

bool BarsParse::parseDynamics(MeasureData* measureData, int /*length*/) {
	Block placeHolder;

	Dynamics* dynamics = new Dynamics();
	measureData->addMusicData(dynamics);

	if( !jump(1) ) { return false; }

	// is playback
	if( !readBuffer(placeHolder, 1) ) { return false; }
	dynamics->setIsPlayback(getHighNibble(placeHolder.toUnsignedInt())!=0x4);

	if( !jump(1) ) { return false; }

	// common
	if( !parseCommonBlock(dynamics) ) { return false; }

	// y offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	dynamics->setYOffset(placeHolder.toInt());

	// dynamics type
	if( !readBuffer(placeHolder, 1) ) { return false; }
	dynamics->setDynamicsType(getLowNibble(placeHolder.toUnsignedInt()));

	// velocity
	if( !readBuffer(placeHolder, 1) ) { return false; }
	dynamics->setVelocity(placeHolder.toUnsignedInt());

	int cursor = ove_->getIsVersion4() ? 4 : 2;

	if( !jump(cursor) ) { return false; }

	return true;
}

bool BarsParse::parseKey(MeasureData* measureData, int /*length*/) {
	Block placeHolder;
	Key* key = measureData->getKey();
	int cursor = ove_->getIsVersion4() ? 9 : 7;

	if( !jump(cursor) ) { return false; }

	// key
	if( !readBuffer(placeHolder, 1) ) { return false; }
	key->setKey(oveKeyToKey(placeHolder.toUnsignedInt()));

	// previous key
	if( !readBuffer(placeHolder, 1) ) { return false; }
	key->setPreviousKey(oveKeyToKey(placeHolder.toUnsignedInt()));

	if( !jump(3) ) { return false; }

	// symbol count
	if( !readBuffer(placeHolder, 1) ) { return false; }
	key->setSymbolCount(placeHolder.toUnsignedInt());

	if( !jump(4) ) { return false; }

	return true;
}

bool BarsParse::parsePedal(MeasureData* measureData, int length) {
	Block placeHolder;

	Pedal* pedal = new Pedal();
	//measureData->addMusicData(pedal); //can't remember why
	measureData->addCrossMeasureElement(pedal, true);

	if( !jump(1) ) { return false; }

	// is playback
	if( !readBuffer(placeHolder, 1) ) { return false; }
	pedal->setIsPlayback(getHighNibble(placeHolder.toUnsignedInt())!=4);

	if( !jump(1) ) { return false; }

	// common
	if( !parseCommonBlock(pedal) ) { return false; }

	if( !jump(2) ) { return false; }

	// pair lines
	if( !parsePairLinesBlock(pedal) ) { return false; }

	// offset common
	if( !parseOffsetCommonBlock(pedal) ) { return false; }

	// left shoulder
	if( !parseOffsetElement(pedal->getLeftShoulder()) ) { return false; }

	// right shoulder
	if( !parseOffsetElement(pedal->getRightShoulder()) ) { return false; }

	int cursor = ove_->getIsVersion4() ? 0x45 : 0x23;
	int blankCount = ove_->getIsVersion4() ? 42 : 10;

	pedal->setHalf( length > cursor );

	if( !jump(blankCount) ) { return false; }

	if( length > cursor ) {
		if( !jump(2) ) { return false; }

		// handle x offset
		if( !readBuffer(placeHolder, 2) ) { return false; }
		pedal->getPedalHandle()->setXOffset(placeHolder.toInt());

		if( !jump(6) ) { return false; }
	}

	return true;
}

bool BarsParse::parseKuohao(MeasureData* measureData, int /*length*/) {
	Block placeHolder;

	KuoHao* kuoHao = new KuoHao();
	measureData->addMusicData(kuoHao);

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(kuoHao) ) { return false; }

	if( !jump(2) ) { return false; }

	// pair lines
	if( !parsePairLinesBlock(kuoHao) ) { return false; }

	if( !jump(4) ) { return false; }

	// left shoulder
	if( !parseOffsetElement(kuoHao->getLeftShoulder()) ) { return false; }

	// right shoulder
	if( !parseOffsetElement(kuoHao->getRightShoulder()) ) { return false; }

	// kuohao type
	if( !readBuffer(placeHolder, 1) ) { return false; }
	kuoHao->setKuohaoType(placeHolder.toUnsignedInt());

	// height
	if( !readBuffer(placeHolder, 1) ) { return false; }
	kuoHao->setHeight(placeHolder.toUnsignedInt());

	int jumpAmount = ove_->getIsVersion4() ? 40 : 8;
	if( !jump(jumpAmount) ) { return false; }

	return true;
}

bool BarsParse::parseExpressions(MeasureData* measureData, int length) {
	Block placeHolder;

	Expressions* expressions = new Expressions();
	measureData->addMusicData(expressions);

	if( !jump(3) ) { return false; }

	// common00
	if( !parseCommonBlock(expressions) ) { return false; }

	if( !jump(2) ) { return false; }

	// y offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	expressions->setYOffset(placeHolder.toInt());

	// range bar offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	//int barOffset = placeHolder.toUnsignedInt();

	if( !jump(10) ) { return false; }

	// tempo 1
	if( !readBuffer(placeHolder, 2) ) { return false; }
	//double tempo1 = ((double)placeHolder.toUnsignedInt()) / 100.0;

	// tempo 2
	if( !readBuffer(placeHolder, 2) ) { return false; }
	//double tempo2 = ((double)placeHolder.toUnsignedInt()) / 100.0;

	if( !jump(6) ) { return false; }

	// text
	int cursor = ove_->getIsVersion4() ? 35 : 33;
	if( length > cursor ) {
		if( !readBuffer(placeHolder, length-cursor) ) { return false; }
		expressions->setText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));
	}

	return true;
}

bool BarsParse::parseHarpPedal(MeasureData* measureData, int /*length*/) {
	Block placeHolder;

	HarpPedal* harpPedal = new HarpPedal();
	measureData->addMusicData(harpPedal);

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(harpPedal) ) { return false; }

	if( !jump(2) ) { return false; }

	// y offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	harpPedal->setYOffset(placeHolder.toInt());

	// show type
	if( !readBuffer(placeHolder, 1) ) { return false; }
	harpPedal->setShowType(placeHolder.toUnsignedInt());

	// show char flag
	if( !readBuffer(placeHolder, 1) ) { return false; }
	harpPedal->setShowCharFlag(placeHolder.toUnsignedInt());

	if( !jump(8) ) { return false; }

	return true;
}

bool BarsParse::parseMultiMeasureRest(MeasureData* measureData, int /*length*/) {
	Block placeHolder(2);
	MultiMeasureRest* measureRest = new MultiMeasureRest();
	measureData->addMusicData(measureRest);

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(measureRest) ) { return false; }

	if( !jump(6) ) { return false; }

	return true;
}

bool BarsParse::parseHarmonyGuitarFrame(MeasureData* measureData, int length) {
	Block placeHolder;

	Harmony* harmony = new Harmony();
	measureData->addMusicData(harmony);

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(harmony) ) { return false; }

	// root
	if( !readBuffer(placeHolder, 1) ) { return false; }
	harmony->setRoot(placeHolder.toUnsignedInt());

	// type
	if( !readBuffer(placeHolder, 1) ) { return false; }
	harmony->setHarmonyType((HarmonyType)placeHolder.toUnsignedInt());

	// bass
	if( !readBuffer(placeHolder, 1) ) { return false; }
	harmony->setBass(placeHolder.toUnsignedInt());

	int jumpAmount = ove_->getIsVersion4() ? length - 12 : length - 10;
	if( !jump(jumpAmount) ) { return false; }

	return true;
}

void extractOctave(unsigned int Bits, OctaveShiftType& octaveShiftType,	QList<OctaveShiftPosition>& positions) {
	octaveShiftType = OctaveShift_8;
	positions.clear();

	switch (Bits) {
	case 0x0: {
		octaveShiftType = OctaveShift_8;
		positions.push_back(OctavePosition_Continue);
		break;
	}
	case 0x1: {
		octaveShiftType = OctaveShift_Minus_8;
		positions.push_back(OctavePosition_Continue);
		break;
	}
	case 0x2: {
		octaveShiftType = OctaveShift_15;
		positions.push_back(OctavePosition_Continue);
		break;
	}
	case 0x3: {
		octaveShiftType = OctaveShift_Minus_15;
		positions.push_back(OctavePosition_Continue);
		break;
	}
	case 0x4: {
		octaveShiftType = OctaveShift_8;
		positions.push_back(OctavePosition_Stop);
		break;
	}
	case 0x5: {
		octaveShiftType = OctaveShift_Minus_8;
		positions.push_back(OctavePosition_Stop);
		break;
	}
	case 0x6: {
		octaveShiftType = OctaveShift_15;
		positions.push_back(OctavePosition_Stop);
		break;
	}
	case 0x7: {
		octaveShiftType = OctaveShift_Minus_15;
		positions.push_back(OctavePosition_Stop);
		break;
	}
	case 0x8: {
		octaveShiftType = OctaveShift_8;
		positions.push_back(OctavePosition_Start);
		break;
	}
	case 0x9: {
		octaveShiftType = OctaveShift_Minus_8;
		positions.push_back(OctavePosition_Start);
		break;
	}
	case 0xA: {
		octaveShiftType = OctaveShift_15;
		positions.push_back(OctavePosition_Start);
		break;
	}
	case 0xB: {
		octaveShiftType = OctaveShift_Minus_15;
		positions.push_back(OctavePosition_Start);
		;
		break;
	}
	case 0xC: {
		octaveShiftType = OctaveShift_8;
		positions.push_back(OctavePosition_Start);
		positions.push_back(OctavePosition_Stop);
		break;
	}
	case 0xD: {
		octaveShiftType = OctaveShift_Minus_8;
		positions.push_back(OctavePosition_Start);
		positions.push_back(OctavePosition_Stop);
		break;
	}
	case 0xE: {
		octaveShiftType = OctaveShift_15;
		positions.push_back(OctavePosition_Start);
		positions.push_back(OctavePosition_Stop);
		break;
	}
	case 0xF: {
		octaveShiftType = OctaveShift_Minus_15;
		positions.push_back(OctavePosition_Start);
		positions.push_back(OctavePosition_Stop);
		break;
	}
	default:
		break;
	}
}

bool BarsParse::parseOctaveShift(MeasureData* measureData, int /*length*/) {
	Block placeHolder;

	OctaveShift* octave = new OctaveShift();
	measureData->addCrossMeasureElement(octave, true);

	if( !jump(3) ) { return false; }

	// common
	if( !parseCommonBlock(octave) ) { return false; }

	// octave
	if( !readBuffer(placeHolder, 1) ) { return false; }
	unsigned int type = getLowNibble(placeHolder.toUnsignedInt());
	OctaveShiftType octaveShiftType = OctaveShift_8;
	QList<OctaveShiftPosition> positions;
	extractOctave(type, octaveShiftType, positions);

	octave->setOctaveShiftType(octaveShiftType);

	if( !jump(1) ) { return false; }

	// y offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	octave->setYOffset(placeHolder.toInt());

	if( !jump(4) ) { return false; }

	// length
	if( !readBuffer(placeHolder, 2) ) { return false; }
	octave->setLength(placeHolder.toUnsignedInt());

	// end tick
	if( !readBuffer(placeHolder, 2) ) { return false; }
	octave->setEndTick(placeHolder.toUnsignedInt());

	// start & stop maybe appear in same measure
	for(unsigned int i=0; i<positions.size(); ++i) {
		OctaveShiftPosition position = positions[i];
		OctaveShiftEndPoint* octavePoint = new OctaveShiftEndPoint();
		measureData->addMusicData(octavePoint);

		octavePoint->copyCommonBlock(*octave);
		octavePoint->setOctaveShiftType(octaveShiftType);
		octavePoint->setOctaveShiftPosition(position);
		octavePoint->setEndTick(octave->getEndTick());

		// stop
		if( i==0 && position == OctavePosition_Stop ) {
			octavePoint->start()->setOffset(octave->start()->getOffset()+octave->getLength());
		}

		// end point
		if( i>0 ) {
			octavePoint->start()->setOffset(octave->start()->getOffset()+octave->getLength());
			octavePoint->setTick(octave->getEndTick());
		}
	}

	return true;
}

bool BarsParse::parseMidiController(MeasureData* measureData, int /*length*/) {
	Block placeHolder;
	MidiController* controller = new MidiController();
	measureData->addMidiData(controller);

	parseMidiCommon(controller);

	// value [0, 128)
	if( !readBuffer(placeHolder, 1) ) { return false; }
	controller->setValue(placeHolder.toUnsignedInt());

	// controller number
	if( !readBuffer(placeHolder, 1) ) { return false; }
	controller->setController(placeHolder.toUnsignedInt());

	if( ove_->getIsVersion4() ) {
		if( !jump(2) ) { return false; }
	}

	return true;
}

bool BarsParse::parseMidiProgramChange(MeasureData* measureData, int /*length*/) {
	Block placeHolder;
	MidiProgramChange* program = new MidiProgramChange();
	measureData->addMidiData(program);

	parseMidiCommon(program);

	if( !jump(1) ) { return false; }

	// patch
	if( !readBuffer(placeHolder, 1) ) { return false; }
	program->setPatch(placeHolder.toUnsignedInt());

	if( ove_->getIsVersion4() ) {
		if( !jump(2) ) { return false; }
	}

	return true;
}

bool BarsParse::parseMidiChannelPressure(MeasureData* measureData, int /*length*/) {
	Block placeHolder;
	MidiChannelPressure* pressure = new MidiChannelPressure();
	measureData->addMidiData(pressure);

	parseMidiCommon(pressure);

	if( !jump(1) ) { return false; }

	// pressure
	if( !readBuffer(placeHolder, 1) ) { return false; }
	pressure->setPressure(placeHolder.toUnsignedInt());

	if( ove_->getIsVersion4() )
	{
		if( !jump(2) ) { return false; }
	}

	return true;
}

bool BarsParse::parseMidiPitchWheel(MeasureData* measureData, int /*length*/) {
	Block placeHolder;
	MidiPitchWheel* wheel = new MidiPitchWheel();
	measureData->addMidiData(wheel);

	parseMidiCommon(wheel);

	// pitch wheel
	if( !readBuffer(placeHolder, 2) ) { return false; }
	int value = placeHolder.toUnsignedInt();
	wheel->setValue(value);

	if( ove_->getIsVersion4() ) {
		if( !jump(2) ) { return false; }
	}

	return true;
}

bool BarsParse::parseSizeBlock(int length) {
	if( !jump(length) ) { return false; }

	return true;
}

bool BarsParse::parseMidiCommon(MidiData* ptr) {
	Block placeHolder;

	if( !jump(3) ) { return false; }

	// start position
	if( !readBuffer(placeHolder, 2) ) { return false; }
	ptr->setTick(placeHolder.toUnsignedInt());

	return true;
}

bool BarsParse::parseCommonBlock(MusicData* ptr) {
	Block placeHolder;

	// start tick
	if( !readBuffer(placeHolder, 2) ) { return false; }
	ptr->setTick(placeHolder.toInt());

	// start unit
	if( !readBuffer(placeHolder, 2) ) { return false; }
	ptr->start()->setOffset(placeHolder.toInt());

	if( ove_->getIsVersion4() ) 	{
		// color
		if( !readBuffer(placeHolder, 1) ) { return false; }
		ptr->setColor(placeHolder.toUnsignedInt());

		if( !jump(1) ) { return false; }
	}

	return true;
}

bool BarsParse::parseOffsetCommonBlock(MusicData* ptr) {
	Block placeHolder;

	// offset measure
	if( !readBuffer(placeHolder, 2) ) { return false; }
	ptr->stop()->setMeasure(placeHolder.toUnsignedInt());

	// end unit
	if( !readBuffer(placeHolder, 2) ) { return false; }
	ptr->stop()->setOffset(placeHolder.toInt());

	return true;
}

bool BarsParse::parsePairLinesBlock(PairEnds* ptr) {
	Block placeHolder;

	// left line
	if( !readBuffer(placeHolder, 2) ) { return false; }
	ptr->getLeftLine()->setLine(placeHolder.toInt());

	// right line
	if( !readBuffer(placeHolder, 2) ) { return false; }
	ptr->getRightLine()->setLine(placeHolder.toInt());

	return true;
}

bool BarsParse::parseOffsetElement(OffsetElement* ptr) {
	Block placeHolder;

	// x offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	ptr->setXOffset(placeHolder.toInt());

	// y offset
	if( !readBuffer(placeHolder, 2) ) { return false; }
	ptr->setYOffset(placeHolder.toInt());

	return true;
}

bool BarsParse::getCondElementType(unsigned int byteData, CondType& type) {
	if( byteData == 0x09 ) {
		type = Cond_Time_Parameters;
	} else if (byteData == 0x0A) {
		type = Cond_Bar_Number;
	} else if (byteData == 0x16) {
		type = Cond_Decorator;
	} else if (byteData == 0x1C) {
		type = Cond_Tempo;
	} else if (byteData == 0x1D) {
		type = Cond_Text;
	} else if (byteData == 0x25) {
		type = Cond_Expression;
	} else if (byteData == 0x30) {
		type = Cond_Barline_Parameters;
	} else if (byteData == 0x31) {
		type = Cond_Repeat;
	} else if (byteData == 0x32) {
		type = Cond_Numeric_Ending;
	} else {
		return false;
	}

	return true;
}

bool BarsParse::getBdatElementType(unsigned int byteData, BdatType& type) {
	if (byteData == 0x70) {
		type = Bdat_Raw_Note;
	} else if (byteData == 0x80) {
		type = Bdat_Rest;
	} else if (byteData == 0x90) {
		type = Bdat_Note;
	} else if (byteData == 0x10) {
		type = Bdat_Beam;
	} else if (byteData == 0x11) {
		type = Bdat_Harmony;
	} else if (byteData == 0x12) {
		type = Bdat_Clef;
	} else if (byteData == 0x13) {
		type = Bdat_Wedge;
	} else if (byteData == 0x14) {
		type = Bdat_Dynamics;
	} else if (byteData == 0x15) {
		type = Bdat_Glissando;
	} else if (byteData == 0x16) {
		type = Bdat_Decorator;
	} else if (byteData == 0x17) {
		type = Bdat_Key;
	} else if (byteData == 0x18) {
		type = Bdat_Lyric;
	} else if (byteData == 0x19) {
		type = Bdat_Octave_Shift;
	} else if (byteData == 0x1B) {
		type = Bdat_Slur;
	} else if (byteData == 0x1D) {
		type = Bdat_Text;
	} else if (byteData == 0x1E) {
		type = Bdat_Tie;
	} else if (byteData == 0x1F) {
		type = Bdat_Tuplet;
	} else if (byteData == 0x21) {
		type = Bdat_Guitar_Bend;
	} else if (byteData == 0x22) {
		type = Bdat_Guitar_Barre;
	} else if (byteData == 0x23) {
		type = Bdat_Pedal;
	} else if (byteData == 0x24) {
		type = Bdat_KuoHao;
	} else if (byteData == 0x25) {
		type = Bdat_Expressions;
	} else if (byteData == 0x26) {
		type = Bdat_Harp_Pedal;
	} else if (byteData == 0x27) {
		type = Bdat_Multi_Measure_Rest;
	} else if (byteData == 0x28) {
		type = Bdat_Harmony_GuitarFrame;
	} else if (byteData == 0x40) {
		type = Bdat_Graphics_40;
	} else if (byteData == 0x41) {
		type = Bdat_Graphics_RoundRect;
	} else if (byteData == 0x42) {
		type = Bdat_Graphics_Rect;
	} else if (byteData == 0x43) {
		type = Bdat_Graphics_Round;
	} else if (byteData == 0x44) {
		type = Bdat_Graphics_Line;
	} else if (byteData == 0x45) {
		type = Bdat_Graphics_Curve;
	} else if (byteData == 0x46) {
		type = Bdat_Graphics_WedgeSymbol;
	} else if (byteData == 0xAB) {
		type = Bdat_Midi_Controller;
	} else if (byteData == 0xAC) {
		type = Bdat_Midi_Program_Change;
	} else if (byteData == 0xAD) {
		type = Bdat_Midi_Channel_Pressure;
	} else if (byteData == 0xAE) {
		type = Bdat_Midi_Pitch_Wheel;
	} else if (byteData == 0xFF) {
		type = Bdat_Bar_End;
	} else {
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
LyricChunkParse::LyricChunkParse(OveSong* ove) :
	BasicParse(ove) {
}

void LyricChunkParse::setLyricChunk(SizeChunk* chunk) {
	chunk_ = chunk;
}

// only ove3 has this chunk
bool LyricChunkParse::parse() {
	unsigned int i;
	Block* dataBlock = chunk_->getDataBlock();
	unsigned int blockSize = chunk_->getSizeBlock()->toSize();
	StreamHandle handle(dataBlock->data(), blockSize);
	Block placeHolder;

	handle_ = &handle;

	if( !jump(4) ) { return false; }

	// Lyric count
	if( !readBuffer(placeHolder, 2) ) { return false; }
	unsigned int count = placeHolder.toUnsignedInt();

	for( i=0; i<count; ++i ) {
		LyricInfo info;

		if( !readBuffer(placeHolder, 2) ) { return false; }
		//unsigned int size = placeHolder.toUnsignedInt();

		// 0x0D00
		if( !jump(2) ) { return false; }

		// voice
		if( !readBuffer(placeHolder, 1) ) { return false; }
		info.voice_ = placeHolder.toUnsignedInt();

		// verse
		if( !readBuffer(placeHolder, 1) ) { return false; }
		info.verse_ = placeHolder.toUnsignedInt();

		// track
		if( !readBuffer(placeHolder, 1) ) { return false; }
		info.track_ = placeHolder.toUnsignedInt();

		if( !jump(1) ) { return false; }

		// measure
		if( !readBuffer(placeHolder, 2) ) { return false; }
		info.measure_ = placeHolder.toUnsignedInt();

		// word count
		if( !readBuffer(placeHolder, 2) ) { return false; }
		info.wordCount_ = placeHolder.toUnsignedInt();

		// lyric size
		if( !readBuffer(placeHolder, 2) ) { return false; }
		info.lyricSize_ = placeHolder.toUnsignedInt();

		if( !jump(6) ) { return false; }

		// name
		if( !readBuffer(placeHolder, 32) ) { return false; }
		info.name_ = ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray());

		if( info.lyricSize_ > 0 ) {
			// lyric
			if( info.lyricSize_ > 0 ) {
				if( !readBuffer(placeHolder, info.lyricSize_) ) { return false; }
				info.lyric_ = ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray());
			}

			if( !jump(4) ) { return false; }

			// font
			if( !readBuffer(placeHolder, 2) ) { return false; }
			info.font_ = placeHolder.toUnsignedInt();

			if( !jump(1) ) { return false; }

			// font size
			if( !readBuffer(placeHolder, 1) ) { return false; }
			info.fontSize_ = placeHolder.toUnsignedInt();

			// font style
			if( !readBuffer(placeHolder, 1) ) { return false; }
			info.fontStyle_ = placeHolder.toUnsignedInt();

			if( !jump(1) ) { return false; }

			for( int j=0; j<info.wordCount_; ++j ) {
				if( !jump(8) ) { return false; }
			}
		}

		processLyricInfo(info);
	}

	return true;
}

bool isSpace(char c) {
	return c == ' ' || c == '\n';
}

void LyricChunkParse::processLyricInfo(const LyricInfo& info) {
	unsigned int i;
	unsigned int j;
	unsigned int index = 0; //words

	int measureId = info.measure_-1;
	bool changeMeasure = true;
	MeasureData* measureData = 0;
	int trackMeasureCount = ove_->getTrackBarCount();
	QStringList words = info.lyric_.split(" ", QString::SkipEmptyParts);

	while ( index < words.size() && measureId+1 < trackMeasureCount ) {
		if( changeMeasure ) {
			++measureId;
			measureData = ove_->getMeasureData(info.track_, measureId);
			changeMeasure = false;
		}

		if( measureData == 0 ) { return; }
		QList<NoteContainer*> containers = measureData->getNoteContainers();
		QList<MusicData*> lyrics = measureData->getMusicDatas(MusicData_Lyric);

		for( i=0; i<containers.size() && index<words.size(); ++i ) {
			if( containers[i]->getIsRest() ) {
				continue;
			}

			for( j=0; j<lyrics.size(); ++j ) {
				Lyric* lyric = static_cast<Lyric*>(lyrics[j]);

				if( containers[i]->start()->getOffset() == lyric->start()->getOffset() &&
					(int)containers[i]->getVoice() == info.voice_ &&
					lyric->getVerse() == info.verse_ ) {
						if(index<words.size()) {
							QString l = words[index].trimmed();
							if(!l.isEmpty()) {
								lyric->setLyric(l);
								lyric->setVoice(info.voice_);
							}
						}

						++index;
				}
			}
		}

		changeMeasure = true;
	}
}

///////////////////////////////////////////////////////////////////////////////
TitleChunkParse::TitleChunkParse(OveSong* ove) :
	BasicParse(ove) {
	titleType_ = 0x00000001;
	annotateType_ = 0x00010000;
	writerType_ = 0x00020002;
	copyrightType_ = 0x00030001;
	headerType_ = 0x00040000;
	footerType_ = 0x00050002;
}

void TitleChunkParse::setTitleChunk(SizeChunk* chunk) {
	chunk_ = chunk;
}

bool TitleChunkParse::parse() {
	Block* dataBlock = chunk_->getDataBlock();
	unsigned int blockSize = chunk_->getSizeBlock()->toSize();
	StreamHandle handle(dataBlock->data(), blockSize);
	Block typeBlock;
	unsigned int titleType;

	handle_ = &handle;

	if( !readBuffer(typeBlock, 4) ) { return false; }

	titleType = typeBlock.toUnsignedInt();

	if( titleType == titleType_ || titleType == annotateType_ || titleType == writerType_ || titleType == copyrightType_ ) {
		Block offsetBlock;

		if( !readBuffer(offsetBlock, 4) ) { return false; }

		const unsigned int itemCount = 4;
		unsigned int i;

		for( i=0; i<itemCount; ++i ) {
			if( i>0 ) {
				//0x 00 AB 00 0C 00 00
				if( !jump(6) ) { return false; }
			}

			Block countBlock;
			if( !readBuffer(countBlock, 2) ) { return false; }
			unsigned int titleSize = countBlock.toUnsignedInt();

			Block dataBlock;
			if( !readBuffer(dataBlock, titleSize) ) { return false; }

			QByteArray array((char*)dataBlock.data(), dataBlock.size());
			addToOve(ove_->getCodecString(array), titleType);
		}

		return true;
	}

	if( titleType == headerType_ || titleType == footerType_ ) {
		if( !jump(10) ) { return false; }

		Block countBlock;
		if( !readBuffer(countBlock, 2) ) { return false; }
		unsigned int titleSize = countBlock.toUnsignedInt();

		Block dataBlock;
		if( !readBuffer(dataBlock, titleSize) ) { return false; }

		QByteArray array((char*)dataBlock.data(), dataBlock.size());
		addToOve(ove_->getCodecString(array), titleType);

		//0x 00 AB 00 0C 00 00
		if( !jump(6) ) { return false; }

		return true;
	}

	return false;
}

void TitleChunkParse::addToOve(const QString& str, unsigned int titleType) {
	if( str.isEmpty() ) { return; }

	if (titleType == titleType_) {
		ove_->addTitle(str);
	}

	if (titleType == annotateType_) {
		ove_->addAnnotate(str);
	}

	if (titleType == writerType_) {
		ove_->addWriter(str);
	}

	if (titleType == copyrightType_) {
		ove_->addCopyright(str);
	}

	if (titleType == headerType_) {
		ove_->addHeader(str);
	}

	if (titleType == footerType_) {
		ove_->addFooter(str);
	}
}

// OveOrganize.cpp
OveOrganizer::OveOrganizer(OveSong* ove) {
	ove_ = ove;
}

void OveOrganizer::organize() {
	if(ove_ == NULL) {
		return;
	}

	organizeTracks();
	organizeAttributes();
	organizeMeasures();
}

void OveOrganizer::organizeAttributes() {
	int i;
	int j;
	unsigned int k;

	// key
	if(ove_->getLineCount() > 0) {
		Line* line = ove_->getLine(0);
		int partBarCount = ove_->getPartBarCount();
		int lastKey = 0;

		if(line != 0){
			for(i=0; i<line->getStaffCount(); ++i) {
				QPair<int, int> partStaff = ove_->trackToPartStaff(i);
				Staff* staff = line->getStaff(i);
				lastKey = staff->getKeyType();

				for(j=0; j<partBarCount; ++j) {
					MeasureData* measureData = ove_->getMeasureData(partStaff.first, partStaff.second, j);

					if(measureData != 0) {
						Key* key = measureData->getKey();

						if( j==0 ) {
							key->setKey(lastKey);
							key->setPreviousKey(lastKey);
						}

						if( !key->getSetKey() ) {
							key->setKey(lastKey);
							key->setPreviousKey(lastKey);
						}
						else {
							if( key->getKey() != lastKey ) {
								lastKey = key->getKey();
							}
						}
					}
				}
			}
		}
	}

	// clef
	if( ove_->getLineCount() > 0 ) {
		Line* line = ove_->getLine(0);
		int partBarCount = ove_->getPartBarCount();
		ClefType lastClefType = Clef_Treble;

		if(line != 0){
			for( i=0; i<line->getStaffCount(); ++i ) {
				QPair<int, int> partStaff = ove_->trackToPartStaff(i);
				Staff* staff = line->getStaff(i);
				lastClefType = staff->getClefType();

				for( j=0; j<partBarCount; ++j )	{
					MeasureData* measureData = ove_->getMeasureData(partStaff.first, partStaff.second, j);

					if(measureData != 0) {
						Clef* clefPtr = measureData->getClef();
						clefPtr->setClefType((int)lastClefType);

						const QList<MusicData*>& clefs = measureData->getMusicDatas(MusicData_Clef);

						for( k=0; k<clefs.size(); ++k ) {
							Clef* clef = static_cast<Clef*>(clefs[k]);
							lastClefType = clef->getClefType();
						}
					}
				}
			}
		}
	}
}

Staff* getStaff(OveSong* ove, int track) {
	if (ove->getLineCount() > 0) {
		Line* line = ove->getLine(0);
		if(line != 0 && line->getStaffCount() > 0) {
			Staff* staff = line->getStaff(track);
			return staff;
		}
	}

	return 0;
}

void OveOrganizer::organizeTracks() {
	unsigned int i;
	//QList<QPair<ClefType, int> > trackChannels;
	QList<Track*> tracks = ove_->getTracks();
	QList<bool> comboStaveStarts;

	for( i=0; i<tracks.size(); ++i ) {
		comboStaveStarts.push_back(false);
	}

	for( i=0; i<tracks.size(); ++i ) {
		Staff* staff = getStaff(ove_, i);
		if(staff != 0) {
			if(staff->getGroupType() == Group_Brace && staff->getGroupStaffCount() == 1 ) {
				comboStaveStarts[i] = true;
			}
		}

		/*if( i < tracks.size() - 1 ) {
			if( tracks[i]->getStartClef() == Clef_Treble &&
				tracks[i+1]->getStartClef() == Clef_Bass &&
				tracks[i]->getChannel() == tracks[i+1]->get_channel() ) {
			}
		}*/
	}

	int trackId = 0;
	QList<int> partStaffCounts;

	while( trackId < (int)tracks.size() ) {
		int partTrackCount = 1;

		if( comboStaveStarts[trackId] ) {
			partTrackCount = 2;
		}

		partStaffCounts.push_back(partTrackCount);
		trackId += partTrackCount;
	}

	ove_->setPartStaffCounts(partStaffCounts);
}

void OveOrganizer::organizeMeasures() {
	int trackBarCount = ove_->getTrackBarCount();

	for( int i=0; i<ove_->getPartCount(); ++i ) {
		int partStaffCount = ove_->getStaffCount(i);

		for( int j=0; j<partStaffCount; ++j ) {
			for( int k=0; k<trackBarCount; ++k ) {
				Measure* measure = ove_->getMeasure(k);
				MeasureData* measureData = ove_->getMeasureData(i, j, k);

				organizeMeasure(i, j, measure, measureData);
			}
		}
	}
}

void OveOrganizer::organizeMeasure(int part, int track, Measure* measure, MeasureData* measureData) {
	// note containers
	organizeContainers(part, track, measure, measureData);

	// single end data
	organizeMusicDatas(part, track, measure, measureData);

	// cross measure elements
	organizeCrossMeasureElements(part, track, measure, measureData);
}

void addToList(QList<int>& list, int number) {
	for(int i=0; i<list.size(); ++i){
		if(list[i] == number){
			return;
		}
	}

	list.push_back(number);
}

void OveOrganizer::organizeContainers(int /*part*/, int /*track*/, Measure* measure, MeasureData* measureData) {
	unsigned int i;
	QList<NoteContainer*> containers = measureData->getNoteContainers();
	int barUnits = measure->getTime()->getUnits();
	QList<int> voices;

	for(i=0; i<containers.size(); ++i){
		int endUnit = barUnits;
		if( i < containers.size() - 1 ) {
			endUnit = containers[i+1]->start()->getOffset();
		}

		containers[i]->stop()->setOffset(endUnit);
		addToList(voices, containers[i]->getVoice());
	}

	// shift voices
	qSort(voices.begin(), voices.end());

	for (i = 0; i < voices.size(); ++i) {
		int voice = voices[i];
		// voice -> i
		for(int j=0; j<(int)containers.size(); ++j) {
			if( (int)containers[j]->getVoice() == voice && containers[j]->getVoice() != i ){
				containers[j]->setVoice(i);
			}
		}
	}
}

void OveOrganizer::organizeMusicDatas(int /*part*/, int /*track*/, Measure* measure, MeasureData* measureData) {
	unsigned int i;
	int barIndex = measure->getBarNumber()->getIndex();
	QList<MusicData*> datas = measureData->getMusicDatas(MusicData_None);

	for(i=0; i<datas.size(); ++i) {
		datas[i]->start()->setMeasure(barIndex);
	}
}

void OveOrganizer::organizeCrossMeasureElements(int part, int track, Measure* measure, MeasureData* measureData) {
	unsigned int i;
	QList<MusicData*> pairs = measureData->getCrossMeasureElements(MusicData_None, MeasureData::PairType_Start);

	for(i=0; i<pairs.size(); ++i) {
		MusicData* pair = pairs[i];

		switch ( pair->getMusicDataType() ) {
		case MusicData_Beam :
		case MusicData_Glissando :
		case MusicData_Slur :
		case MusicData_Tie :
		case MusicData_Tuplet :
		case MusicData_Pedal :
		case MusicData_Numeric_Ending :
		//case MusicData_OctaveShift_EndPoint :
		case MusicData_Measure_Repeat :	{
				organizePairElement(pair, part, track, measure, measureData);
				break;
			}
		case MusicData_OctaveShift : {
				OctaveShift* octave = static_cast<OctaveShift*>(pair);
				organizeOctaveShift(octave, measure, measureData);
				break;
			}
		case MusicData_Wedge : {
				Wedge* wedge = static_cast<Wedge*>(pair);
				organizeWedge(wedge, part, track, measure, measureData);
				break;
			}
		default:
			break;
		}
	}
}

void OveOrganizer::organizePairElement(
						MusicData* data,
						int part,
						int track,
						Measure* measure,
						MeasureData* measureData) {
	int bar1Index = measure->getBarNumber()->getIndex();
	int bar2Index = bar1Index + data->stop()->getMeasure();
	MeasureData* measureData2 = ove_->getMeasureData(part, track, bar2Index);

	data->start()->setMeasure(bar1Index);

	if(measureData2 != 0 && measureData != measureData2) {
		measureData2->addCrossMeasureElement(data, false);
	}

	if( data->getMusicDataType() == MusicData_Tuplet ){
		Tuplet* tuplet = static_cast<Tuplet*>(data);
		const QList<NoteContainer*> containers = measureData->getNoteContainers();

		for(unsigned int i=0; i<containers.size(); ++i){
			if(containers[i]->getTick() > tuplet->getTick()){
				break;
			}

			if(containers[i]->getTick() == tuplet->getTick()){
				tuplet->setNoteType(containers[i]->getNoteType());
			}
		}

		int tupletTick = NoteTypeToTick(tuplet->getNoteType(), ove_->getQuarter())*tuplet->getSpace();
		if( tuplet->getTick() % tupletTick != 0 ) {
			int newStartTick = (tuplet->getTick() / tupletTick) * tupletTick;

			for(unsigned int i=0; i<containers.size(); ++i){
				if( containers[i]->getTick() == newStartTick &&
					containers[i]->getTuplet() == tuplet->getTuplet()) {
					tuplet->setTick(containers[i]->getTick());
					tuplet->start()->setOffset(containers[i]->start()->getOffset());
				}
			}
		}
	}
}

void OveOrganizer::organizeOctaveShift(
						OctaveShift* octave,
						Measure* measure,
						MeasureData* measureData) {
	// octave shift
	unsigned int i;
	const QList<NoteContainer*> containers = measureData->getNoteContainers();
	int barIndex = measure->getBarNumber()->getIndex();

	octave->start()->setMeasure(barIndex);

	for(i=0; i<containers.size(); ++i) {
		int noteShift = octave->getNoteShift();
		int containerTick = containers[i]->getTick();

		if( octave->getTick() <= containerTick && octave->getEndTick() > containerTick )	{
			containers[i]->setNoteShift(noteShift);
		}
	}
}

bool getMiddleUnit(
			OveSong* ove, int /*part*/, int /*track*/,
			Measure* measure1, Measure* measure2, int unit1, int /*unit2*/,
			Measure* middleMeasure, int& middleUnit) {
	QList<int> barUnits;
	unsigned int i;
	int bar1Index = measure1->getBarNumber()->getIndex();
	int bar2Index = measure2->getBarNumber()->getIndex();
	int sumUnit = 0;

	for( int j=bar1Index; j<=bar2Index; ++j ) {
		Measure* measure = ove->getMeasure(j);
		barUnits.push_back(measure->getTime()->getUnits());
		sumUnit += measure->getTime()->getUnits();
	}

	int currentSumUnit = 0;
	for( i=0; i<barUnits.size(); ++i ) {
		int barUnit = barUnits[i];

		if( i==0 ) {
			barUnit = barUnits[i] - unit1;
		}

		if( currentSumUnit + barUnit < sumUnit/2 ) {
			currentSumUnit += barUnit;
		}
		else {
			break;
		}
	}

	if( i < barUnits.size() ) {
		int barMiddleIndex = bar1Index + i;
		middleMeasure = ove->getMeasure(barMiddleIndex);
		middleUnit = sumUnit/2 - currentSumUnit;

		return true;
	}

	return false;
}

void OveOrganizer::organizeWedge(Wedge* wedge, int part, int track, Measure* measure, MeasureData* measureData) {
	int bar1Index = measure->getBarNumber()->getIndex();
	int bar2Index = bar1Index + wedge->stop()->getMeasure();
	MeasureData* measureData2 = ove_->getMeasureData(part, track, bar2Index);
	WedgeType wedgeType = wedge->getWedgeType();

	if( wedge->getWedgeType() == Wedge_Double_Line ) {
		wedgeType = Wedge_Cres_Line;
	}

	wedge->start()->setMeasure(bar1Index);

	WedgeEndPoint* startPoint = new WedgeEndPoint();
	measureData->addMusicData(startPoint);

	startPoint->setTick(wedge->getTick());
	startPoint->start()->setOffset(wedge->start()->getOffset());
	startPoint->setWedgeStart(true);
	startPoint->setWedgeType(wedgeType);
	startPoint->setHeight(wedge->getHeight());

	WedgeEndPoint* stopPoint = new WedgeEndPoint();

	stopPoint->setTick(wedge->getTick());
	stopPoint->start()->setOffset(wedge->stop()->getOffset());
	stopPoint->setWedgeStart(false);
	stopPoint->setWedgeType(wedgeType);
	stopPoint->setHeight(wedge->getHeight());

	if(measureData2 != 0) {
		measureData2->addMusicData(stopPoint);
	}

	if( wedge->getWedgeType() == Wedge_Double_Line ) {
		Measure* middleMeasure = NULL;
		int middleUnit = 0;

		getMiddleUnit(
			ove_, part, track,
			measure, ove_->getMeasure(bar2Index),
			wedge->start()->getOffset(), wedge->stop()->getOffset(),
			middleMeasure, middleUnit);

		if( middleMeasure != 0 ) {
			WedgeEndPoint* midStopPoint = new WedgeEndPoint();
			measureData->addMusicData(midStopPoint);

			midStopPoint->setTick(wedge->getTick());
			midStopPoint->start()->setOffset(middleUnit);
			midStopPoint->setWedgeStart(false);
			midStopPoint->setWedgeType(Wedge_Cres_Line);
			midStopPoint->setHeight(wedge->getHeight());

			WedgeEndPoint* midStartPoint = new WedgeEndPoint();
			measureData->addMusicData(midStartPoint);

			midStartPoint->setTick(wedge->getTick());
			midStartPoint->start()->setOffset(middleUnit);
			midStartPoint->setWedgeStart(true);
			midStartPoint->setWedgeType(Wedge_Decresc_Line);
			midStartPoint->setHeight(wedge->getHeight());
		}
	}
}


// OveSerialize.cpp
enum ChunkType {
	Chunk_OVSC = 00 ,
	Chunk_TRKL,
	Chunk_TRAK,
	Chunk_PAGL,
	Chunk_PAGE,
	Chunk_LINL,
	Chunk_LINE,
	Chunk_STAF,
	Chunk_BARL,
	Chunk_MEAS,
	Chunk_COND,
	Chunk_BDAT,
	Chunk_PACH,
	Chunk_FNTS,
	Chunk_ODEV,
	Chunk_TITL,
	Chunk_ALOT,
	Chunk_ENGR,
	Chunk_FMAP,
	Chunk_PCPR,

	// Overture 3.6
	Chunk_LYRC,

	Chunk_NONE
};

ChunkType nameToChunkType(const NameBlock& name) {
	ChunkType type = Chunk_NONE;

	if (name.isEqual("OVSC")) {
		type = Chunk_OVSC;
	} else if (name.isEqual("TRKL")) {
		type = Chunk_TRKL;
	} else if (name.isEqual("TRAK")) {
		type = Chunk_TRAK;
	} else if (name.isEqual("PAGL")) {
		type = Chunk_PAGL;
	} else if (name.isEqual("PAGE")) {
		type = Chunk_PAGE;
	} else if (name.isEqual("LINL")) {
		type = Chunk_LINL;
	} else if (name.isEqual("LINE")) {
		type = Chunk_LINE;
	} else if (name.isEqual("STAF")) {
		type = Chunk_STAF;
	} else if (name.isEqual("BARL")) {
		type = Chunk_BARL;
	} else if (name.isEqual("MEAS")) {
		type = Chunk_MEAS;
	} else if (name.isEqual("COND")) {
		type = Chunk_COND;
	} else if (name.isEqual("BDAT")) {
		type = Chunk_BDAT;
	} else if (name.isEqual("PACH")) {
		type = Chunk_PACH;
	} else if (name.isEqual("FNTS")) {
		type = Chunk_FNTS;
	} else if (name.isEqual("ODEV")) {
		type = Chunk_ODEV;
	} else if (name.isEqual("TITL")) {
		type = Chunk_TITL;
	} else if (name.isEqual("ALOT")) {
		type = Chunk_ALOT;
	} else if (name.isEqual("ENGR")) {
		type = Chunk_ENGR;
	} else if (name.isEqual("FMAP")) {
		type = Chunk_FMAP;
	} else if (name.isEqual("PCPR")) {
		type = Chunk_PCPR;
	} else if (name.isEqual("LYRC")) {
		type = Chunk_LYRC;
	}

	return type;
}

int chunkTypeToMaxTimes(ChunkType type) {
	int maxTimes = -1; // no limit

	switch (type) {
	case Chunk_OVSC: {
		maxTimes = 1;
		break;
	}
	case Chunk_TRKL: {//	case Chunk_TRAK :
		maxTimes = 1;
		break;
	}
	case Chunk_PAGL: {//	case Chunk_PAGE :
		maxTimes = 1;
		break;
	}
//	case Chunk_LINE :
//	case Chunk_STAF :
	case Chunk_LINL: {
		maxTimes = 1;
		break;
	}
//	case Chunk_MEAS :
//	case Chunk_COND :
//	case Chunk_BDAT :
	case Chunk_BARL: {
		maxTimes = 1;
		break;
	}
	case Chunk_PACH:
	case Chunk_FNTS:
	case Chunk_ODEV:
	case Chunk_ALOT:
	case Chunk_ENGR:
	case Chunk_FMAP:
	case Chunk_PCPR: {
		maxTimes = 1;
		break;
	}
	case Chunk_TITL: {
		maxTimes = 8;
		break;
	}
	case Chunk_LYRC: {
		maxTimes = 1;
		break;
	}
//	case Chunk_NONE :
	default:
		break;
	}

	return maxTimes;
}

///////////////////////////////////////////////////////////////////////////////////////////

OveSerialize::OveSerialize() :
	ove_(0),
	streamHandle_(0),
	notify_(0) {
}

OveSerialize::~OveSerialize() {
	if(streamHandle_ != 0) {
		delete streamHandle_;
		streamHandle_ = 0;
	}
}

void OveSerialize::setOve(OveSong* ove) {
	ove_ = ove;
}

void OveSerialize::setFileStream(unsigned char* buffer, unsigned int size) {
	streamHandle_ = new StreamHandle(buffer, size);
}

void OveSerialize::setNotify(IOveNotify* notify) {
	notify_ = notify;
}

void OveSerialize::messageOutError() {
	if (notify_ != NULL) {
		notify_->loadError();
	}
}

void OveSerialize::messageOut(const QString& str) {
	if (notify_ != NULL) {
		notify_->loadInfo(str);
	}
}

bool OveSerialize::load(void) {
	if(streamHandle_ == 0)
		return false;

	if( !readHeader() ) {
		messageOutError();
		return false;
	}

	unsigned int i;
	QMap<ChunkType, int> chunkTimes;
	//bool firstEnter = true;

	for( i=(int)Chunk_OVSC; i<(int)Chunk_NONE; ++i ) {
		chunkTimes[(ChunkType)i] = 0;
	}

	ChunkType chunkType = Chunk_NONE;

	do {
		NameBlock nameBlock;
		SizeChunk sizeChunk;

		if( !readNameBlock(nameBlock) ) { return false; }

		chunkType = nameToChunkType(nameBlock);
		++chunkTimes[chunkType];
		int maxTime = chunkTypeToMaxTimes(chunkType);

		if( maxTime > 0 && chunkTimes[chunkType] > maxTime ) {
			messageOut("format not support, chunk appear more than accept.");
			return false;
		}

		switch (chunkType) {
		/*case Chunk_OVSC :
		 {
		 if( !readHeadData(&sizeChunk) )
		 {
		 messageOut_error();
		 return false;
		 }

		 break;
		 }*/
		case Chunk_TRKL: {
			if (!readTracksData()) {
				messageOutError();
				return false;
			}

			break;
		}
		case Chunk_PAGL: {
			if (!readPagesData()) {
				messageOutError();
				return false;
			}

			break;
		}
		case Chunk_LINL: {
			if (!readLinesData()) {
				messageOutError();
				return false;
			}

			break;
		}
		case Chunk_BARL: {
			if (!readBarsData()) {
				messageOutError();
				return false;
			}

			break;
		}
		case Chunk_TRAK:
		case Chunk_PAGE:
		case Chunk_LINE:
		case Chunk_STAF:
		case Chunk_MEAS:
		case Chunk_COND:
		case Chunk_BDAT: {
			return false;
			break;
		}
		case Chunk_LYRC: {
			SizeChunk lyricChunk;
			if (!readSizeChunk(&lyricChunk)) {
				messageOutError();
				return false;
			}

			LyricChunkParse parse(ove_);

			parse.setLyricChunk(&lyricChunk);
			parse.parse();

			break;
		}
		case Chunk_TITL: {
			SizeChunk titleChunk;
			if (!readSizeChunk(&titleChunk)) {
				messageOutError();
				return false;
			}

			TitleChunkParse titleChunkParse(ove_);

			titleChunkParse.setTitleChunk(&titleChunk);
			titleChunkParse.parse();

			break;
		}
		case Chunk_PACH:
		case Chunk_FNTS:
		case Chunk_ODEV:
		case Chunk_ALOT:
		case Chunk_ENGR:
		case Chunk_FMAP:
		case Chunk_PCPR: {
			if (!readSizeChunk(&sizeChunk)) {
				messageOutError();
				return false;
			}

			break;
		}
		default:
			/*if( firstEnter )
			 {
			 QString info = "Not compatible file, try to load and save with newer version, Overture 4 is recommended.";
			 messageOut(info);
			 messageOutError();

			 return false;
			 }*/

			break;
		}

		//firstEnter = false;
	}
	while ( chunkType != Chunk_NONE );

//	if( !readOveEnd() ) { return false; }

	// organize OveData
	OVE::OveOrganizer organizer(ove_);
	organizer.organize();

	return true;
}

void OveSerialize::release() {
	delete this;
}

bool OveSerialize::readHeader() {
	ChunkType chunkType = Chunk_NONE;
	NameBlock nameBlock;
	SizeChunk sizeChunk;

	if (!readNameBlock(nameBlock)) {
		return false;
	}

	chunkType = nameToChunkType(nameBlock);
	//int maxTime = chunkTypeToMaxTimes(chunkType);

	if (chunkType == Chunk_OVSC) {
		if (readHeadData(&sizeChunk)) {
			return true;
		}
	}

	QString info = "Not compatible file, try to load and save with newer version, Overture 4 is recommended.";
	messageOut(info);

	return false;
}

bool OveSerialize::readHeadData(SizeChunk* ovscChunk) {
	if (!readSizeChunk(ovscChunk))
		return false;

	OvscParse ovscParse(ove_);

	ovscParse.setNotify(notify_);
	ovscParse.setOvsc(ovscChunk);

	return ovscParse.parse();
}

bool OveSerialize::readTracksData() {
	GroupChunk trackGroupChunk;

	if (!readGroupChunk(&trackGroupChunk))
		return false;

	unsigned int i;
	unsigned short trackCount = trackGroupChunk.getCountBlock()->toCount();

	for (i = 0; i < trackCount; ++i) {
		SizeChunk* trackChunk = new SizeChunk();

		if (ove_->getIsVersion4()) {
			if (!readChunkName(trackChunk, Chunk::TrackName)) {
				return false;
			}
			if (!readSizeChunk(trackChunk)) {
				return false;
			}
		} else {
			if (!readDataChunk(trackChunk->getDataBlock(),
					SizeChunk::version3TrackSize)) {
				return false;
			}
		}

		TrackParse trackParse(ove_);

		trackParse.setTrack(trackChunk);
		trackParse.parse();
	}

	return true;
}

bool OveSerialize::readPagesData() {
	GroupChunk pageGroupChunk;

	if (!readGroupChunk(&pageGroupChunk))
		return false;

	unsigned short pageCount = pageGroupChunk.getCountBlock()->toCount();
	unsigned int i;
	PageGroupParse parse(ove_);

	for (i = 0; i < pageCount; ++i) {
		SizeChunk* pageChunk = new SizeChunk();

		if (!readChunkName(pageChunk, Chunk::PageName)) {
			return false;
		}
		if (!readSizeChunk(pageChunk)) {
			return false;
		}

		parse.addPage(pageChunk);
	}

	if (!parse.parse()) {
		return false;
	}

	return true;
}

bool OveSerialize::readLinesData() {
	GroupChunk lineGroupChunk;
	if (!readGroupChunk(&lineGroupChunk))
		return false;

	unsigned short lineCount = lineGroupChunk.getCountBlock()->toCount();
	unsigned int i;
	unsigned int j;
	QList<SizeChunk*> lineChunks;
	QList<SizeChunk*> staffChunks;

	for (i = 0; i < lineCount; ++i) {
		SizeChunk* lineChunk = new SizeChunk();

		if (!readChunkName(lineChunk, Chunk::LineName)) {
			return false;
		}
		if (!readSizeChunk(lineChunk)) {
			return false;
		}

		lineChunks.push_back(lineChunk);

		StaffCountGetter getter(ove_);
		unsigned int staffCount = getter.getStaffCount(lineChunk);

		for (j = 0; j < staffCount; ++j) {
			SizeChunk* staffChunk = new SizeChunk();

			if (!readChunkName(staffChunk, Chunk::StaffName)) {
				return false;
			}
			if (!readSizeChunk(staffChunk)) {
				return false;
			}

			staffChunks.push_back(staffChunk);
		}
	}

	LineGroupParse parse(ove_);

	parse.setLineGroup(&lineGroupChunk);

	for (i = 0; i < lineChunks.size(); ++i) {
		parse.addLine(lineChunks[i]);
	}

	for (i = 0; i < staffChunks.size(); ++i) {
		parse.addStaff(staffChunks[i]);
	}

	if (!parse.parse()) {
		return false;
	}

	return true;
}

bool OveSerialize::readBarsData() {
	GroupChunk barGroupChunk;
	if (!readGroupChunk(&barGroupChunk))
		return false;

	unsigned short measCount = barGroupChunk.getCountBlock()->toCount();
	int i;

	QList<SizeChunk*> measureChunks;
	QList<SizeChunk*> conductChunks;
	QList<SizeChunk*> bdatChunks;

	ove_->setTrackBarCount(measCount);

	// read chunks
	for (i = 0; i < measCount; ++i) {
		SizeChunk* measureChunkPtr = new SizeChunk();

		if (!readChunkName(measureChunkPtr, Chunk::MeasureName)) {
			return false;
		}
		if (!readSizeChunk(measureChunkPtr)) {
			return false;
		}

		measureChunks.push_back(measureChunkPtr);
	}

	for (i = 0; i < measCount; ++i) {
		SizeChunk* conductChunkPtr = new SizeChunk();

		if (!readChunkName(conductChunkPtr, Chunk::ConductName))
			return false;

		if (!readSizeChunk(conductChunkPtr))
			return false;

		conductChunks.push_back(conductChunkPtr);
	}

	int bdatCount = ove_->getTrackCount() * measCount;
	for (i = 0; i < bdatCount; ++i) {
		SizeChunk* batChunkPtr = new SizeChunk();

		if (!readChunkName(batChunkPtr, Chunk::BdatName)) {
			return false;
		}
		if (!readSizeChunk(batChunkPtr)) {
			return false;
		}

		bdatChunks.push_back(batChunkPtr);
	}

	// parse bars
	BarsParse barsParse(ove_);

	for (i = 0; i < (int) measureChunks.size(); ++i) {
		barsParse.addMeasure(measureChunks[i]);
	}

	for (i = 0; i < (int) conductChunks.size(); ++i) {
		barsParse.addConduct(conductChunks[i]);
	}

	for (i = 0; i < (int) bdatChunks.size(); ++i) {
		barsParse.addBdat(bdatChunks[i]);
	}

	barsParse.setNotify(notify_);
	if (!barsParse.parse()) {
		return false;
	}

	return true;
}

bool OveSerialize::readOveEnd() {
	if (streamHandle_ == 0)
		return false;

	const unsigned int END_OVE1 = 0xffffffff;
	const unsigned int END_OVE2 = 0x00000000;
	unsigned int buffer;

	if (!streamHandle_->read((char*) &buffer, sizeof(unsigned int)))
		return false;

	if (buffer != END_OVE1)
		return false;

	if (!streamHandle_->read((char*) &buffer, sizeof(unsigned int)))
		return false;

	if (buffer != END_OVE2)
		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool OveSerialize::readNameBlock(NameBlock& nameBlock) {
	if (streamHandle_ == 0)
		return false;

	if (!streamHandle_->read((char*) nameBlock.data(), nameBlock.size()))
		return false;

	return true;
}

bool OveSerialize::readChunkName(Chunk* /*chunk*/, const QString& name) {
	if (streamHandle_ == 0)
		return false;

	NameBlock nameBlock;

	if (!streamHandle_->read((char*) nameBlock.data(), nameBlock.size()))
		return false;

	if (!(nameBlock.toStrByteArray() == name))
		return false;

	return true;
}

bool OveSerialize::readSizeChunk(SizeChunk* sizeChunk) {
	if (streamHandle_ == 0)
		return false;

	SizeBlock* sizeBlock = sizeChunk->getSizeBlock();

	if (!streamHandle_->read((char*) sizeBlock->data(), sizeBlock->size()))
		return false;

	unsigned int blockSize = sizeBlock->toSize();

	sizeChunk->getDataBlock()->resize(blockSize);

	Block* dataBlock = sizeChunk->getDataBlock();

	if (!streamHandle_->read((char*) dataBlock->data(), blockSize))
		return false;

	return true;
}

bool OveSerialize::readDataChunk(Block* block, unsigned int size) {
	if (streamHandle_ == 0)
		return false;

	block->resize(size);

	if (!streamHandle_->read((char*) block->data(), size))
		return false;

	return true;
}

bool OveSerialize::readGroupChunk(GroupChunk* groupChunk) {
	if (streamHandle_ == 0)
		return false;

	CountBlock* countBlock = groupChunk->getCountBlock();

	if (!streamHandle_->read((char*) countBlock->data(), countBlock->size()))
		return false;

	return true;
}

IOVEStreamLoader* createOveStreamLoader() {
	return new OveSerialize;
}

} // end of OVE namespace

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace drumstick {

class MeasureToTick {
public:
	MeasureToTick();
	virtual ~MeasureToTick() {
	}

public:
	void build(OVE::OveSong* ove, int quarter);

	struct TimeTick {
		int numerator_;
		int denominator_;
		int measure_;
		int tick_;

		TimeTick() :
			numerator_(4), denominator_(4), measure_(0), tick_(0) {
		}
	};
	int getTick(int measure, int tickOffset);
	QList<TimeTick> getTimeTicks() const;

private:
	int quarter_;
	OVE::OveSong* ove_;

	QList<TimeTick> tts_;
};

int getMeasureTick(int quarter, int num, int den) {
	return quarter * 4 * num / den;
}

MeasureToTick::MeasureToTick() {
	quarter_ = 480;
	ove_ = NULL;
}

void MeasureToTick::build(OVE::OveSong* ove, int quarter) {
	unsigned int i;
	int currentTick = 0;
	unsigned int measureCount = ove->getMeasureCount();

	quarter_ = quarter;
	ove_ = ove;
	tts_.clear();

	for (i = 0; i < measureCount; ++i) {
		OVE::Measure* measure = ove_->getMeasure(i);
		OVE::TimeSignature* time = measure->getTime();
		TimeTick tt;
		bool change = false;

		tt.tick_ = currentTick;
		tt.numerator_ = time->getNumerator();
		tt.denominator_ = time->getDenominator();
		tt.measure_ = i;

		if (i == 0) {
			change = true;
		} else {
			OVE::TimeSignature* previousTime = ove_->getMeasure(i - 1)->getTime();

			if (time->getNumerator() != previousTime->getNumerator()
					|| time->getDenominator() != previousTime->getDenominator()) {
				change = true;
			}
		}

		if (change) {
			tts_.push_back(tt);
		}

		currentTick += getMeasureTick(quarter_, tt.numerator_, tt.denominator_);
	}
}

int MeasureToTick::getTick(int measure, int tickOffset) {
	unsigned int i;
	TimeTick tt;

	for (i = 0; i < tts_.size(); ++i) {
		if (measure >= tts_[i].measure_ && (i == tts_.size() - 1 || measure < tts_[i + 1].measure_)) {
			int measuresTick = (measure - tts_[i].measure_) *
					getMeasureTick(quarter_, tts_[i].numerator_, tts_[i].denominator_);

			return tts_[i].tick_ + measuresTick + tickOffset;
		}
	}

	return 0;
}

QList<MeasureToTick::TimeTick> MeasureToTick::getTimeTicks() const {
	return tts_;
}

/////////////////////////////////////////////////////////////////////////////////////////
class QOve::QOvePrivate {
public:
    QOvePrivate() {}

    ~QOvePrivate() {}

    OVE::OveSong ove;
    MeasureToTick mtt;
};

QOve::QOve(QObject * parent) :
	QObject(parent),
	d(new QOvePrivate)
{
}

QOve::~QOve() {
	delete d;
}

void QOve::setTextCodecName(const QString& codec) {
	d->ove.setTextCodecName(codec);
}

void QOve::readFromFile(const QString& fileName) {
	QFile oveFile(fileName);
	bool success = true;

	if (oveFile.open(QFile::ReadOnly)) {
		QByteArray buffer = oveFile.readAll();

		oveFile.close();
		d->ove.clear();

		// ove -> OveSong
		OVE::IOVEStreamLoader* oveLoader = OVE::createOveStreamLoader();

		oveLoader->setOve(&d->ove);
		oveLoader->setFileStream((unsigned char*) buffer.data(), buffer.size());
		oveLoader->setNotify(0);
		bool result = oveLoader->load();
		oveLoader->release();

		if(!result)
			success = false;

		if (result) {
			convertSong();
		}
	}

	if(!success) {
		Q_EMIT signalOVEError(
				"Can't read OVE file or not compatible file, \n"
				"try to load and save with newer version, Overture 4 is recommended.");
	}
}

void QOve::convertSong() {
	unsigned int i;
	int trackNo = 0;

	d->mtt.build(&d->ove, d->ove.getQuarter());

	Q_EMIT signalOVEHeader(d->ove.getQuarter(), d->ove.getTrackCount());

	convertSignatures();

	for (i = 0; i < (unsigned int) d->ove.getPartCount(); ++i) {
		int partStaffCount = d->ove.getStaffCount(i);

		for (int j = 0; j < partStaffCount; ++j) {
			OVE::Track* trackPtr = d->ove.getTrack(i, j);
			int transpose = trackPtr->getShowTranspose() ? trackPtr->getTranspose() : 0;

			convertTrackHeader(trackPtr, trackNo);

			int beginMeasure = 0;
			int endMeasure = d->ove.getMeasureCount();
			int offsetTick = 0;

			for (int l = 0; l < endMeasure; ++l) {
				OVE::Measure* measure = d->ove.getMeasure(l);
				OVE::MeasureData* measureData = d->ove.getMeasureData(i, j, l);

/*				if (notify_ != NULL) {
					notify_->notify_convert_pos(l, endMeasure, trackID, d->ove_.getPartCount());
				}*/

				convertMeasure(trackPtr, trackNo, trackPtr->getVoices(), measure, measureData, transpose, offsetTick);
			}

			++trackNo;
		}
	}

	Q_EMIT signalOVEEnd();
}

void QOve::convertSignatures() {
	int i;
	int beginMeasure = 0;
	int endMeasure = d->ove.getMeasureCount();

	// tempo
	QMap<int, int> tempos;
	for (i = 0; i < d->ove.getPartCount(); ++i) {
		if(i>0) {
			break;
		}

		int partStaffCount = d->ove.getStaffCount(i);

		for (int j = 0; j < partStaffCount; ++j) {
			if(j>0) {
				break;
			}

			for (int k = beginMeasure; k < endMeasure; ++k) {
				OVE::Measure* measure = d->ove.getMeasure(k);
				OVE::MeasureData* measureData = d->ove.getMeasureData(i, j, k);
				QList<OVE::MusicData*> tempoPtrs = measureData->getMusicDatas(OVE::MusicData_Tempo);

				if (k == 0 || (k > 0 && abs(measure->getTypeTempo()	- d->ove.getMeasure(k - 1)->getTypeTempo()) > 0.01)) {
					int tick = d->mtt.getTick(k, 0);
					int tempo = (int) measure->getTypeTempo();
					tempos[tick] = tempo;
				}

				for (unsigned int l = 0; l < tempoPtrs.size(); ++l) {
					OVE::Tempo* ptr = static_cast<OVE::Tempo*> (tempoPtrs[l]);
					int tick = d->mtt.getTick(measure->getBarNumber()->getIndex(), ptr->getTick());
					int tempo = ptr->getQuarterTempo() > 0 ? ptr->getQuarterTempo() : 1;

					tempos[tick] = tempo;
				}
			}
		}
	}

	QMap<int, int>::iterator it;
	int lastTempo = 0;
	for (it = tempos.begin(); it != tempos.end(); ++it) {
		if (it == tempos.begin() || it.value() != lastTempo) {
			Q_EMIT signalOVETempo(it.key(), it.value()*100);
		}

		lastTempo = it.value();
	}

	// time signature
	const QList<MeasureToTick::TimeTick> tts = d->mtt.getTimeTicks();
	for (i = 0; i < (int) tts.size(); ++i) {
		if (beginMeasure <= tts[i].measure_ && endMeasure > tts[i].measure_) {
			Q_EMIT signalOVETimeSig(tts[i].measure_, tts[i].tick_, tts[i].numerator_, tts[i].denominator_);
		}
	}

	// key signature
	bool createKey = false;
	for (i = beginMeasure; i < endMeasure; ++i) {
		OVE::MeasureData* measureData = d->ove.getMeasureData(0, 0, i);

		if (measureData != NULL) {
			OVE::Key* keyPtr = measureData->getKey();

			if (i == 0 || keyPtr->getKey() != keyPtr->getPreviousKey()) {
				Q_EMIT signalOVEKeySig(i, d->mtt.getTick(i, 0), keyPtr->getKey());

				createKey = true;
			}
		}
	}

	if (!createKey) {
		Q_EMIT signalOVEKeySig(0, 0, 0);
	}
}

void QOve::convertTrackHeader(OVE::Track* track, int trackNo) {
	unsigned int i;
	const QList<OVE::Voice*> voices = track->getVoices();
	QMap<int, int> patches;	// channel, patch
	QMap<int, int> pans;	// channel, pan
	QMap<int, int> volumes;	// channel, volume
	QMap<int, int>::iterator it;
	int ch = 0;
	int vol = 100;
	int patch = 0;

	// name
	QString trackName = track->getName();

	// patch, pan(control 10), volume(control 7)
	for (i = 0; i < voices.size() && i < (unsigned int)track->getVoiceCount(); ++i) {
		int patch = voices[i]->getPatch();
		int channel = voices[i]->getChannel();
		int volume = voices[i]->getVolume();

		if (patch != OVE::Voice::getDefaultPatch()) {
			patches[channel] = patch;
		}

		pans[channel] = voices[i]->getPan();

		if (volume != OVE::Voice::getDefaultVolume()) {
			volumes[channel] = volume;
		}

		ch = channel;
		vol = volume;
	}

	// patch
	for (it = patches.begin(); it != patches.end(); ++it) {
		patch = it.value();
		break;
	}

	// pan
	int lastPan = 64;//center
	for (it = pans.begin(); it != pans.end(); ++it) {
		if (it.value() != 0 && it.value() != lastPan) {
			Q_EMIT signalOVECtlChange(trackNo, 0, ch, 10, it.value());
		}

		lastPan = it.value();
	}

	// volume
	for (it = volumes.begin(); it != volumes.end(); ++it) {
		int volume = it.value();
		if (volume != -1) {
			Q_EMIT signalOVECtlChange(trackNo, 0, ch, 7, it.value());
		}
	}

	Q_EMIT signalOVENewTrack(track->getName(), trackNo, ch, 0, vol, 0, false, false, false);
	Q_EMIT signalOVETrackBank(trackNo, ch, 0);
	Q_EMIT signalOVETrackPatch(trackNo, ch, patch);
}

int getPitchShift(const QList<OVE::Voice*>& voices, int voice) {
	if (voice >= 0 && voice < (int) voices.size())
		return voices[voice]->getPitchShift();
	return 0;
}

int getChannel(const QList<OVE::Voice*>& voices, int voice) {
	if (voice >= 0 && voice < (int) voices.size())
		return voices[voice]->getChannel();
	return 0;
}

int getTick(int tick) {
	if(tick > 0)
		return tick;
	return 0;
}

void QOve::convertMeasure(
		OVE::Track* track, int trackNo, const QList<OVE::Voice*>& voices,
		OVE::Measure* measure, OVE::MeasureData* measureData, int transpose, int offsetTick) {
	unsigned int i;
	unsigned int j;
	int measureId = measure->getBarNumber()->getIndex();
	QList<OVE::NoteContainer*> containers = measureData->getNoteContainers();

	// midi notes
	for (i = 0; i < containers.size(); ++i) {
		OVE::NoteContainer* container = containers[i];
		int measureTick = d->mtt.getTick(measureId, 0);
		convertNotes(
				trackNo,
				measureTick,
				container,
				getChannel(voices, container->getVoice()),
				getPitchShift(voices, container->getVoice()) - transpose);
	}

	// midi events in graph window
	QList<OVE::MidiData*> midiDatas = measureData->getMidiDatas(OVE::Midi_None);
	int channel = getChannel(voices, 0);

	for (i = 0; i < midiDatas.size(); ++i) {
		OVE::MidiType midiType = midiDatas[i]->getMidiType();
		int midiTick = getTick(d->mtt.getTick(measureId, midiDatas[i]->getTick()) + offsetTick);

		switch (midiType) {
		case OVE::Midi_Controller: {
			OVE::MidiController* controller = static_cast<OVE::MidiController*> (midiDatas[i]);

			Q_EMIT signalOVECtlChange(trackNo, midiTick, channel, controller->getController(), controller->getValue());

			break;
		}
		case OVE::Midi_Program_Change: {
			OVE::MidiProgramChange* program = static_cast<OVE::MidiProgramChange*> (midiDatas[i]);

			Q_EMIT signalOVEProgram(trackNo, midiTick, channel, program->getPatch());

			break;
		}
		case OVE::Midi_Channel_Pressure: {
			OVE::MidiChannelPressure* pressure = static_cast<OVE::MidiChannelPressure*> (midiDatas[i]);

			Q_EMIT signalOVEChanPress(trackNo, midiTick, channel, pressure->getPressure());

			break;
		}
		case OVE::Midi_Pitch_Wheel: {
			OVE::MidiPitchWheel* pitchWheel = static_cast<OVE::MidiPitchWheel*> (midiDatas[i]);

			Q_EMIT signalOVEPitchBend(trackNo, midiTick, channel, pitchWheel->getValue());

			break;
		}
		default:
			break;
		}
	}

	// MusicData
	QList<OVE::MusicData*> musicDatas = measureData->getMusicDatas(OVE::MusicData_None);

	for (i = 0; i < musicDatas.size(); ++i) {
		OVE::MusicDataType type = musicDatas[i]->getMusicDataType();
		int musicDataTick = getTick(d->mtt.getTick(measureId, musicDatas[i]->getTick())	+ offsetTick);

		switch (type) {
		case OVE::MusicData_Lyric: {
			OVE::Lyric* lyricPtr = static_cast<OVE::Lyric*> (musicDatas[i]);

			Q_EMIT signalOVEText(trackNo, musicDataTick, lyricPtr->getLyric());

			break;
		}
		case OVE::MusicData_Dynamics: {
			OVE::Dynamics* dynamicPtr = static_cast<OVE::Dynamics*> (musicDatas[i]);

			Q_EMIT signalOVECtlChange(trackNo, musicDataTick, channel, 7, dynamicPtr->getVelocity());

			break;
		}
		case OVE::MusicData_Decorator: {
			OVE::Decorator* decorator = static_cast<OVE::Decorator*> (musicDatas[i]);

			if (decorator->getDecoratorType() == OVE::Decorator::Decorator_Articulation) {
				OVE::ArticulationType artType = decorator->getArticulationType();

				switch (artType) {
				case OVE::Articulation_Pedal_Down: {
					Q_EMIT signalOVECtlChange(trackNo, musicDataTick, channel, 64, 64);

					break;
				}
				case OVE::Articulation_Pedal_Up: {
					Q_EMIT signalOVECtlChange(trackNo, musicDataTick, channel, 64, 0);

					break;
				}
				default:
					break;
				}
			}

			break;
		}
		default:
			break;
		}
	}
}

bool hasNoteOn(int pos) {
	return (pos & OVE::Tie_RightEnd) != OVE::Tie_RightEnd;
}

bool hasNoteOff(int pos) {
	return (pos & OVE::Tie_LeftEnd) != OVE::Tie_LeftEnd;
}

int noteTypeToTick(OVE::NoteType type, int quarter = 480) {
	int c = int(pow(2.0, (int) type));
	return quarter * 4 * 2 / c;
}

void QOve::convertNotes(int trackNo, int measureTick, OVE::NoteContainer* container, int channel, int pitchShift) {
	if (container->getIsRest()) {
		return;
	}

	unsigned int i;
	unsigned int j;
	int k;
	QList<OVE::Note*> notes = container->getNotesRests();
	QList<OVE::Articulation*> articulations = container->getArticulations();
	bool changeNoteCount = false;

	// for those who can change note numbers
	for (i = 0; i < articulations.size(); ++i) {
		OVE::Articulation* art = articulations[i];
		OVE::ArticulationType type = art->getArtType();

		for (j = 0; j < notes.size(); ++j) {
			OVE::Note* notePtr = notes[j];
			unsigned int velocityValue = notePtr->getOnVelocity();
			int noteValue = notePtr->getNote() + container->getNoteShift() + pitchShift;
			int startTick = measureTick + container->getTick() + notePtr->getOffsetTick();

			// note on
			if (hasNoteOn(notePtr->getTiePos())) {
				switch (type) {
				// tremolo
				case OVE::Articulation_Tremolo_Eighth:
				case OVE::Articulation_Tremolo_Sixteenth:
				case OVE::Articulation_Tremolo_Thirty_Second:
				case OVE::Articulation_Tremolo_Sixty_Fourth: {
					int noteCount = (int) pow(2.0, ((int) type - (int) OVE::Articulation_Tremolo_Eighth) + 1);
					int noteTick = noteTypeToTick(container->getNoteType(), d->ove.getQuarter()) / noteCount;

					for (k = 0; k < noteCount; ++k) {
						// on
						int onTick = getTick(startTick + k * noteTick);

						Q_EMIT signalOVENoteOn(trackNo, onTick, channel, noteValue, velocityValue);

						if (k < noteCount - 1 || hasNoteOff((int) notePtr->getTiePos())) {
							// off
							int offTick = getTick(startTick + (k + 1) * noteTick);

							Q_EMIT signalOVENoteOff(trackNo, offTick, channel, noteValue, velocityValue);
						}
					}

					changeNoteCount = true;

					break;
				}
				default:
					break;
				}
			}
		}
	}

	if (changeNoteCount) {
		return;
	}

	// others who can change note properties
	for (i = 0; i < notes.size(); ++i) {
		OVE::Note* notePtr = notes[i];
		unsigned int velocityValue = notePtr->getOnVelocity();
		int noteValue = notePtr->getNote() + container->getNoteShift() + pitchShift;

		int startTick = getTick(measureTick + container->getTick() + notePtr->getOffsetTick());
		int lengthTick = container->getLength();

		// note on
		if (hasNoteOn((int) notePtr->getTiePos())) {
			for (j = 0; j < articulations.size(); ++j) {
				OVE::Articulation* art = articulations[j];
				OVE::ArticulationType type = art->getArtType();

				if (art->getChangeLength()) {
					lengthTick = noteTypeToTick(container->getNoteType(), d->ove.getQuarter()) * art->getLengthPercentage() / 100;
				}

				if (art->getChangeVelocity()) {
					switch (art->getVelocityType()) {
					case OVE::Articulation::Velocity_Offset: {
						velocityValue += art->getVelocityValue();
						break;
					}
					case OVE::Articulation::Velocity_Percentage: {
						velocityValue *= (unsigned int) ((double) art->getVelocityValue() / (double) 100);
						break;
					}
					case OVE::Articulation::Velocity_SetValue: {
						velocityValue = art->getVelocityValue();
						break;
					}
					default:
						break;
					}
				}

				if (art->getChangeExtraLength()) {
				}

				switch (type) {
				case OVE::Articulation_Pedal_Down: {
					Q_EMIT signalOVECtlChange(trackNo, startTick, channel, 64, 64);

					break;
				}
				case OVE::Articulation_Pedal_Up: {
					Q_EMIT signalOVECtlChange(trackNo, startTick, channel, 64, 0);

					break;
				}
				case OVE::Articulation_Arpeggio: {
					//if( art->getChangeSoundEffect() ) {
						unsigned int soundEffect = abs(art->getSoundEffect().first) + abs(art->getSoundEffect().second);
						int tickAmount = (soundEffect / notes.size()) * ((notes.size() - i) - 1);
						startTick -= tickAmount;
					//}

					break;
				}
				default:
					break;
				}
			}

			Q_EMIT signalOVENoteOn(trackNo, getTick(startTick), channel, noteValue, velocityValue);
		}

		// note off
		if (hasNoteOff(notePtr->getTiePos())) {
			Q_EMIT signalOVENoteOff(trackNo, getTick(startTick + lengthTick), channel, noteValue, velocityValue);
		}
	}

	return;
}

}
