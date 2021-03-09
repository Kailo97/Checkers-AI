#pragma once
#include "ICheckersPlayer.h"
#include "ICheckersPlayerBehavior.h"
#include <array>
#include <list>
#include <optional>
#include <tuple>
#include <functional>
#include "Util.h"

class CCheckers;
class CCheckersPiece;

class CCheckersTile
{
public:
	using neighbors_container = std::array<std::reference_wrapper<CCheckersTile>, 6>;
	using neighbors_iterator = neighbors_container::const_iterator;
	using neighbors_iterator_pair = iterator_pair<neighbors_iterator>;

	CCheckersTile(CheckersColumn column, CheckersRow row, CCheckersTile& frontright, CCheckersTile& frontleft,
		CCheckersTile& backleft, CCheckersTile& backright, const size_t &offset, bool edge);

	CheckersTilePos Pos() const;
	bool Empty() const;
	bool Busy() const;
	bool Outside() const;

	//inline bool RealEmpty() const
	//{
	//	return !Outside() && Empty();
	//}
	//inline bool RealBusy() const
	//{
	//	return !Outside() && Busy();
	//}

	std::optional<std::reference_wrapper<const CCheckersPiece>> Piece() const;
	void SetPiece(CCheckersPiece& piece);
	void RemovePiece();

private:
	friend class CCheckers;

	CCheckersPiece& GetPiece() const;
	CheckersTilePos pos;
	std::optional<std::reference_wrapper<CCheckersPiece>> piece;

private:
	const size_t &offset;

	// frontright_neighbor
	// frontleft_neighbor
	// backleft_neighbor
	// backright_neighbor
	// frontright_neighbor
	// frontleft_neighbor
	neighbors_container neighbors;

public:
	auto Near() const;
	auto Front() const;

	std::reference_wrapper<CCheckersTile> FrontRight() const;
	std::reference_wrapper<CCheckersTile> FrontLeft() const;
	std::reference_wrapper<CCheckersTile> BackLeft() const;
	std::reference_wrapper<CCheckersTile> BackRight() const;

public:
	bool IsCrown() const;

private:
	std::function<bool(void)> crown_rule;
	bool NonCrowned() const;
	bool Crowned() const;
};

class CCheckersMove
{
public:
	CCheckersMove(CCheckersPiece& mover, CCheckersTile& from, CCheckersTile& to);
	CCheckersMove(CCheckersPiece& mover, CCheckersTile& from, CCheckersTile& to, CCheckersPiece& captured);

	const CCheckersTile& From() const;
	const CCheckersTile& To() const;
	const CCheckersPiece& Mover() const;
	bool Capture() const;
	size_t CaptureCount() const;
	bool SingleCapture() const;
	bool MultiplyCapture() const;

	size_t MovesCount() const;
	bool SingleMove() const;
	bool MultiplyMove() const;

	std::list<std::reference_wrapper<CCheckersPiece>>::const_iterator cptr_begin() const;
	std::list<std::reference_wrapper<CCheckersPiece>>::const_iterator cptr_end() const;

private:
	friend class CCheckers;

	void AddCapture(CCheckersTile& dist, CCheckersPiece& captured);

	void Perform() const;

	CCheckersPiece& mover;
	std::list<std::reference_wrapper<CCheckersTile>> moves;
	std::list<std::reference_wrapper<CCheckersPiece>> capture;

	// DEV: bad
	friend class CheckersNetPlayer;
};

class CCheckersPlayer;

class CCheckersPiece
{
public:
	CCheckersPiece(const CCheckersPlayer& owner, CCheckersTile& place);

	bool IsKing() const;
	void ToKing();
	void Captured();
	void OnCaptured(std::function<void()>&& oncapture);
	
	const CCheckersTile& Place() const;
	void MoveTo(CCheckersTile& place);
	const CCheckersPlayer &Owner() const;
	CheckersPlayerColor Color() const;

private:
	friend class CCheckers;

	const CCheckersPlayer& owner;
	std::reference_wrapper<CCheckersTile> place;
	std::function<void(void)> oncapture;
	bool king;
};

using move_container = std::list<CCheckersMove>;

using CCheckersMovesIt = move_container::const_iterator;

using CCheckersPlayerBehavior = ICheckersPlayerBehavior<CCheckers, CCheckersMovesIt>;

class CCheckersPlayer : public ICheckersPlayer
{
public:
	CCheckersPlayer(const CCheckersPlayerBehavior& behavior, const CCheckersPlayer& enemy);

public:
	const CCheckersPlayer& Enemy() const override;

private:
	friend class CCheckers;

	void Piece(CCheckersTile& place);
	const std::list<CCheckersPiece>& Pieces();

private:
	const CCheckersPlayerBehavior& behavior;
	const CCheckersPlayer& enemy;
	std::list<CCheckersPiece> pieces;
};

class CCheckers
{
public:
	CCheckers(const CCheckersPlayerBehavior &white, const CCheckersPlayerBehavior &black);
	~CCheckers();

	// TODO: Checkers maybe inited in any position
	//void Init();
	void InitToDefault();

	// Return winner
	const CCheckersPlayer &Play();

	using tilesit = std::array<CCheckersTile, 32>::const_iterator;

	tilesit tbegin() const;
	tilesit tend() const;

private:
	void Init(std::initializer_list<std::reference_wrapper<CCheckersTile>> whites, std::initializer_list<std::reference_wrapper<CCheckersTile>> blacks);

	// Moves deduction
	static move_container join_moves(move_container a, move_container b);
	template<class T>
	static move_container accumulete_moves(move_container a, const T& v, const std::function<move_container(const T&)>& action);

	static std::list<std::pair<std::reference_wrapper<CCheckersTile>, std::reference_wrapper<CCheckersPiece>>>
		piece_eat_more(const CCheckersTile& origin, CheckersPlayerColor color, const CCheckersTile& ignore, const std::function<CCheckersTile& (const CCheckersTile&)>& getnext);
	static std::list<std::pair<std::reference_wrapper<CCheckersTile>, std::reference_wrapper<CCheckersPiece>>>
		paths_to_eat(const CCheckersPiece& mover, CCheckersTile& origin);
	static void piece_move_feeder(move_container& m, move_container::iterator mit);
	static void piece_move_feeder(move_container& m);

private:
	friend class CCheckersPlayer;

	class WhitePlayer : public CCheckersPlayer
	{
	public:
		using CCheckersPlayer::CCheckersPlayer;

		CheckersPlayerColor Color() const override;
	} white;

	class BlackPlayer : public CCheckersPlayer
	{
	public:
		using CCheckersPlayer::CCheckersPlayer;

		CheckersPlayerColor Color() const override;
	} black;

	std::tuple<std::reference_wrapper<CCheckersPlayer>, size_t> now;
	std::tuple<std::reference_wrapper<CCheckersPlayer>, size_t> next;

	std::reference_wrapper<CCheckersPlayer>& now_player;
	size_t& now_offset;
	std::reference_wrapper<CCheckersPlayer>& next_player;

private:
	friend class CheckersBoardAdaptor;

	static CCheckersTile OOB;
	friend bool CCheckersTile::Outside() const;
	union
	{
		//  [A1, C1, E1, G1, B2, D2 ... F8, H8 ]
		std::array<CCheckersTile, 32> board;
		struct
		{
			CCheckersTile A1; CCheckersTile C1; CCheckersTile E1; CCheckersTile G1;
			CCheckersTile B2; CCheckersTile D2; CCheckersTile F2; CCheckersTile H2;
			CCheckersTile A3; CCheckersTile C3; CCheckersTile E3; CCheckersTile G3;
			CCheckersTile B4; CCheckersTile D4; CCheckersTile F4; CCheckersTile H4;
			CCheckersTile A5; CCheckersTile C5; CCheckersTile E5; CCheckersTile G5;
			CCheckersTile B6; CCheckersTile D6; CCheckersTile F6; CCheckersTile H6;
			CCheckersTile A7; CCheckersTile C7; CCheckersTile E7; CCheckersTile G7;
			CCheckersTile B8; CCheckersTile D8; CCheckersTile F8; CCheckersTile H8;
		};
	};
	
public:
	void print() const;

	// TODO: Log of moves
	
};