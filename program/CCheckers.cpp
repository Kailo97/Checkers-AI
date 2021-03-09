#include "CCheckers.h"

#include <algorithm>
#include <execution>
#include <iostream>

CCheckersTile::CCheckersTile(CheckersColumn column, CheckersRow row, CCheckersTile& frontright, CCheckersTile& frontleft,
	CCheckersTile& backleft, CCheckersTile& backright, const size_t& offset, bool edge)
	: pos{ row, column }, piece{ std::nullopt }, offset(offset), crown_rule{std::bind(edge ? &CCheckersTile::Crowned : &CCheckersTile::NonCrowned, this)},
		neighbors{ std::ref(frontright), std::ref(frontleft), std::ref(backleft), std::ref(backright),std::ref(frontright), std::ref(frontleft) }
{
}

CheckersTilePos CCheckersTile::Pos() const
{
	return pos;
}

bool CCheckersTile::Empty() const
{
	return piece == std::nullopt;
}

bool CCheckersTile::Busy() const
{
	return piece != std::nullopt;
}

bool CCheckersTile::Outside() const
{
	return this == &CCheckers::OOB;
}

std::optional<std::reference_wrapper<const CCheckersPiece>> CCheckersTile::Piece() const
{
	return piece;
}

void CCheckersTile::SetPiece(CCheckersPiece& p)
{
	piece = std::ref(p);
}

void CCheckersTile::RemovePiece()
{
	piece = std::nullopt;
}

CCheckersPiece& CCheckersTile::GetPiece() const
{
	return *piece;
}

auto CCheckersTile::Near() const
{
	return neighbors_iterator_pair{ neighbors.cbegin() + offset, neighbors.cbegin() + 4 + offset };
}

auto CCheckersTile::Front() const
{
	return neighbors_iterator_pair{ neighbors.cbegin() + offset, neighbors.cbegin() + 2 + offset };
}

std::reference_wrapper<CCheckersTile> CCheckersTile::FrontRight() const
{
	return neighbors.at(0 + offset);
}

std::reference_wrapper<CCheckersTile> CCheckersTile::FrontLeft() const
{
	return neighbors.at(1 + offset);
}

std::reference_wrapper<CCheckersTile> CCheckersTile::BackLeft() const
{
	return neighbors.at(2 + offset);
}

std::reference_wrapper<CCheckersTile> CCheckersTile::BackRight() const
{
	return neighbors.at(3 + offset);
}

bool CCheckersTile::IsCrown() const
{
	return crown_rule();
}

bool CCheckersTile::NonCrowned() const
{
	return false;
}

bool CCheckersTile::Crowned() const
{
	return !offset;
}


CCheckersMove::CCheckersMove(CCheckersPiece& mover, CCheckersTile& from, CCheckersTile& to)
	: mover{ mover }, moves{ from, to }
{

}

CCheckersMove::CCheckersMove(CCheckersPiece& mover, CCheckersTile& from, CCheckersTile& to, CCheckersPiece& captured)
	: mover{ mover }, moves{ from, to }, capture{ captured }
{
}

const CCheckersTile& CCheckersMove::From() const
{
	return moves.front().get();
}

const CCheckersTile& CCheckersMove::To() const
{
	return moves.back().get();
}

const CCheckersPiece& CCheckersMove::Mover() const
{
	return mover;
}

bool CCheckersMove::Capture() const
{
	return !capture.empty();
}

size_t CCheckersMove::CaptureCount() const
{
	return capture.size();
}

bool CCheckersMove::SingleCapture() const
{
	return capture.size() == 1;
}

bool CCheckersMove::MultiplyCapture() const
{
	return capture.size() > 1;
}

size_t CCheckersMove::MovesCount() const
{
	return moves.size() - 1;
}

bool CCheckersMove::SingleMove() const
{
	return moves.size() == 1;
}

bool CCheckersMove::MultiplyMove() const
{
	return moves.size() > 1;
}

std::list<std::reference_wrapper<CCheckersPiece>>::const_iterator CCheckersMove::cptr_begin() const
{
	return capture.cbegin();
}

std::list<std::reference_wrapper<CCheckersPiece>>::const_iterator CCheckersMove::cptr_end() const
{
	return capture.cend();
}

void CCheckersMove::AddCapture(CCheckersTile& dist, CCheckersPiece& captured)
{
	moves.emplace_back(dist);
	capture.emplace_back(captured);
}

void CCheckersMove::Perform() const
{
	if (!capture.empty())
		std::for_each(std::begin(capture), std::end(capture), [](CCheckersPiece& piece) {
			piece.Captured();
		});
	mover.MoveTo(moves.back());
}


CCheckersPiece::CCheckersPiece(const CCheckersPlayer& owner, CCheckersTile& place)
	: owner{ owner }, place{ place }, king( false )
{
	place.SetPiece(*this);
}

bool CCheckersPiece::IsKing() const
{
	return king;
}

void CCheckersPiece::ToKing()
{
	king = true;
}

void CCheckersPiece::Captured()
{
	place.get().RemovePiece();
	oncapture();
}

void CCheckersPiece::OnCaptured(std::function<void()>&& oncapture)
{
	CCheckersPiece::oncapture = oncapture;
}

const CCheckersTile& CCheckersPiece::Place() const
{
	return place;
}

void CCheckersPiece::MoveTo(CCheckersTile& place)
{
	CCheckersPiece::place.get().RemovePiece();
	place.SetPiece(*this);
	CCheckersPiece::place = place;
}

const CCheckersPlayer& CCheckersPiece::Owner() const
{
	return owner;
}

CheckersPlayerColor CCheckersPiece::Color() const
{
	return owner.Color();
}


CCheckersPlayer::CCheckersPlayer(const CCheckersPlayerBehavior& behavior, const CCheckersPlayer& enemy)
	: behavior(behavior), enemy(enemy)
{
}

const CCheckersPlayer& CCheckersPlayer::Enemy() const
{
	return enemy;
}

void CCheckersPlayer::Piece(CCheckersTile& place)
{
	pieces.emplace_back(*this, place);
	using piece_container = std::list<CCheckersPiece>;
	using list_erase_fn = piece_container::iterator(piece_container::const_iterator);
	pieces.back().OnCaptured(std::bind<void>(std::mem_fn<list_erase_fn>(&piece_container::erase), std::ref(pieces), std::prev(pieces.cend())));
}

const std::list<CCheckersPiece>& CCheckersPlayer::Pieces()
{
	return pieces;
}


CCheckersTile CCheckers::OOB = CCheckersTile(CheckersColumn::O, CheckersRow::Zero, OOB, OOB, OOB, OOB, 0, false);

CCheckers::CCheckers(const CCheckersPlayerBehavior& white_bhv, const CCheckersPlayerBehavior& black_bhv)
	: now_player(std::get<0>(now)), now_offset(std::get<size_t>(now)), next_player(std::get<0>(next)),
	white(white_bhv, black), black(black_bhv, white), now{ std::ref(white), 0 }, next{ std::ref(black), 2 }, 
	board{
		CCheckersTile(CheckersColumn::A, CheckersRow::One,    B2, OOB, OOB, OOB, now_offset, true),
		CCheckersTile(CheckersColumn::C, CheckersRow::One,    D2,  B2, OOB, OOB, now_offset, true),
		CCheckersTile(CheckersColumn::E, CheckersRow::One,    F2,  D2, OOB, OOB, now_offset, true),
		CCheckersTile(CheckersColumn::G, CheckersRow::One,    H2,  F2, OOB, OOB, now_offset, true),
		CCheckersTile(CheckersColumn::B, CheckersRow::Two,    C3,  A3,  A1,  C1, now_offset, false),
		CCheckersTile(CheckersColumn::D, CheckersRow::Two,    E3,  C3,  C1,  E1, now_offset, false),
		CCheckersTile(CheckersColumn::F, CheckersRow::Two,    G3,  E3,  E1,  G1, now_offset, false),
		CCheckersTile(CheckersColumn::H, CheckersRow::Two,   OOB,  G3,  G1, OOB, now_offset, false),
		CCheckersTile(CheckersColumn::A, CheckersRow::Three,  B4, OOB, OOB,  B2, now_offset, false),
		CCheckersTile(CheckersColumn::C, CheckersRow::Three,  D4,  B4,  B2,  D2, now_offset, false),
		CCheckersTile(CheckersColumn::E, CheckersRow::Three,  F4,  D4,  D2,  F2, now_offset, false),
		CCheckersTile(CheckersColumn::G, CheckersRow::Three,  H4,  F4,  F2,  H2, now_offset, false),
		CCheckersTile(CheckersColumn::B, CheckersRow::Four,   C5,  A5,  A3,  C3, now_offset, false),
		CCheckersTile(CheckersColumn::D, CheckersRow::Four,   E5,  C5,  C3,  E3, now_offset, false),
		CCheckersTile(CheckersColumn::F, CheckersRow::Four,   G5,  E5,  E3,  G3, now_offset, false),
		CCheckersTile(CheckersColumn::H, CheckersRow::Four,  OOB,  G5,  G3, OOB, now_offset, false),
		CCheckersTile(CheckersColumn::A, CheckersRow::Five,   B6, OOB, OOB,  B4, now_offset, false),
		CCheckersTile(CheckersColumn::C, CheckersRow::Five,   D6,  B6,  B4,  D4, now_offset, false),
		CCheckersTile(CheckersColumn::E, CheckersRow::Five,   F6,  D6,  D4,  F4, now_offset, false),
		CCheckersTile(CheckersColumn::G, CheckersRow::Five,   H6,  F6,  F4,  H4, now_offset, false),
		CCheckersTile(CheckersColumn::B, CheckersRow::Six,    C7,  A7,  A5,  C5, now_offset, false),
		CCheckersTile(CheckersColumn::D, CheckersRow::Six,    E7,  C7,  C5,  E5, now_offset, false),
		CCheckersTile(CheckersColumn::F, CheckersRow::Six,    G7,  E7,  E5,  G5, now_offset, false),
		CCheckersTile(CheckersColumn::H, CheckersRow::Six,   OOB,  G7,  G5, OOB, now_offset, false),
		CCheckersTile(CheckersColumn::A, CheckersRow::Seven,  B8, OOB, OOB,  B6, now_offset, false),
		CCheckersTile(CheckersColumn::C, CheckersRow::Seven,  D8,  B8,  B6,  D6, now_offset, false),
		CCheckersTile(CheckersColumn::E, CheckersRow::Seven,  F8,  D8,  D6,  F6, now_offset, false),
		CCheckersTile(CheckersColumn::G, CheckersRow::Seven,  H8,  F8,  F6,  H6, now_offset, false),
		CCheckersTile(CheckersColumn::B, CheckersRow::Eight, OOB, OOB,  A7,  C7, now_offset, true),
		CCheckersTile(CheckersColumn::D, CheckersRow::Eight, OOB, OOB,  C7,  E7, now_offset, true),
		CCheckersTile(CheckersColumn::F, CheckersRow::Eight, OOB, OOB,  E7,  G7, now_offset, true),
		CCheckersTile(CheckersColumn::H, CheckersRow::Eight, OOB, OOB,  G7, OOB, now_offset, true),
	}
{
}

CCheckers::~CCheckers()
{
	std::destroy(std::begin(board), std::end(board));
}

void CCheckers::InitToDefault()
{
	Init(
		{ A3, C3, E3, G3, B2, D2, F2, H2, A1, C1, E1, G1 },
		{ B8, D8, F8, H8, A7, C7, E7, G7, B6, D6, F6, H6 }
	);
}

template<class T>
std::list<T> join_lists(std::list<T> a, std::list<T> b)
{
	a.splice(a.cend(), std::move(b));
	return a;
}

move_container CCheckers::join_moves(move_container a, move_container b)
{
	auto capture = [](const move_container& c) -> bool { return c.front().Capture(); };
	if (!a.empty() && !b.empty() && (!capture(a) && capture(b) || capture(a) && !capture(b)))
	{
		if (!capture(a))
			a = std::move(b);
	}
	else
		a.splice(a.cend(), std::move(b));
	return a;
}

template<class T>
move_container CCheckers::accumulete_moves(move_container a, const T& v, const std::function<move_container(const T&)>& action)
{
	return join_moves(std::move(a), std::move(action(v)));
};

std::list<std::pair<std::reference_wrapper<CCheckersTile>, std::reference_wrapper<CCheckersPiece>>>
CCheckers::piece_eat_more(const CCheckersTile& origin, CheckersPlayerColor color, const CCheckersTile& ignore, const std::function<CCheckersTile& (const CCheckersTile&)>& getnext)
{
	const CCheckersTile& next = getnext(origin);
	if (!next.Outside() && !next.Empty())
	{
		CCheckersPiece& neighbor = next.GetPiece();
		if (neighbor.Color() != color)
		{
			CCheckersTile& next_next = getnext(next);
			if (!next_next.Outside() && (next_next.Empty() || std::addressof(next_next) == std::addressof(ignore)))
				return { { std::ref(next_next), std::ref(neighbor) } };
		}
	}
	return {};
}

std::list<std::pair<std::reference_wrapper<CCheckersTile>, std::reference_wrapper<CCheckersPiece>>>
CCheckers::paths_to_eat(const CCheckersPiece& mover, CCheckersTile& origin)
{
	std::initializer_list<std::function<CCheckersTile&(const CCheckersTile&)>>&& l = {
		&CCheckersTile::FrontRight,
		&CCheckersTile::FrontLeft,
		&CCheckersTile::BackLeft,
		&CCheckersTile::BackRight,
	};

	auto p = std::bind(CCheckers::piece_eat_more, std::cref(origin), mover.Color(), std::cref(mover.Place()), std::placeholders::_1);
	return std::accumulate(std::begin(l), std::end(l), std::list<std::pair<std::reference_wrapper<CCheckersTile>, std::reference_wrapper<CCheckersPiece>>>{},
		[&p](auto a, const std::function<CCheckersTile& (const CCheckersTile&)>& dir) -> std::list<std::pair<std::reference_wrapper<CCheckersTile>, std::reference_wrapper<CCheckersPiece>>>
		{ return join_lists(std::move(a), std::move(p(dir))); }
	);
}

void CCheckers::piece_move_feeder(move_container &m, move_container::iterator mit)
{
	// If 0 variants to move next - exit
	// If more than 1, adds copy of mit move right after m
	// Make transform of both sequences to join
	// Contain search in deapth

	auto& captured = mit->capture;
	auto paths = paths_to_eat(mit->mover, mit->moves.back());
	std::erase_if(paths, [&captured](const std::pair<std::reference_wrapper<CCheckersTile>, std::reference_wrapper<CCheckersPiece>>& v) -> bool {
		auto feed = v.second;
		auto cond = [feed](const std::reference_wrapper<CCheckersPiece>& m) { return std::addressof(m.get()) == std::addressof(feed.get()); };
		return std::find_if(std::begin(captured), std::end(captured), cond) != std::end(captured);
	});
	if (paths.empty())
		return;
	move_container::iterator end = std::next(mit);
	if (paths.size() > 1)
	{
		end = m.insert(end, paths.size() - 1, *mit);
		std::advance(end, paths.size() - 1);
	}
	auto pit = std::begin(paths);
	std::for_each(mit, end, [&pit](move_container::reference v) { v.moves.push_back(pit->first); v.capture.push_back(pit->second); ++pit; });
	piece_move_feeder(m, mit);
}

void CCheckers::piece_move_feeder(move_container& m)
{
	for (auto it = m.begin(); it != m.end(); it = std::next(it))
		piece_move_feeder(m, it);
}

const CCheckersPlayer& CCheckers::Play()
{
	// Piece moves deduction
	// 1. TODO

	// Moves accumulateing
	// 1. TODO

	// Play process
	// 1. Deduct piece possible moves for each direction of piece
	// 2. Accumulete to piecemoves
	// 3. Deduct possible moves for each pirce
	// 3. Accumulete to player moves

	std::function piece_possible_moves = [](CCheckersPiece& P) -> move_container {
		std::function piece_direction_moves = [](CCheckersPiece& mover, std::function<CCheckersTile& (const CCheckersTile&)> getnext, bool only_capture) -> move_container {
			move_container m;
			if (!mover.IsKing())
			{
				CCheckersTile& origin = mover.place;
				CCheckersTile& next = getnext(origin);
				if (!next.Outside())
				{
					if (!next.Empty())
					{
						CCheckersPiece& neighbor = next.GetPiece();
						if (neighbor.Color() != mover.Color())
						{
							CCheckersTile& next_next = getnext(next);
							if (!next_next.Outside() && next_next.Empty())
							{
								m.emplace_back(mover, origin, next_next, neighbor);
								// Eat them all!
								piece_move_feeder(m);
							}
						}
					}
					else if (!only_capture)
						m.emplace_back(mover, origin, next);
				}
			}
			return std::move(m);
		};
		std::initializer_list<std::pair<std::function<CCheckersTile& (const CCheckersTile&)>, bool>> &&l = {
			{ &CCheckersTile::FrontRight, false },
			{ &CCheckersTile::FrontLeft, false },
			{ &CCheckersTile::BackLeft, true },
			{ &CCheckersTile::BackRight, true }
		};
		std::function F = [&P, &piece_direction_moves](const std::pair<std::function<CCheckersTile& (const CCheckersTile&)>, bool>& v) -> move_container { return piece_direction_moves(P, std::get<0>(v), std::get<1>(v)); };
		auto A = std::bind<move_container>(accumulete_moves<std::pair<std::function<CCheckersTile& (const CCheckersTile&)>, bool>>, std::placeholders::_1, std::placeholders::_2, std::cref(F));
		return std::accumulate(std::begin(l), std::end(l), move_container(), A);
	};
	
	std::function player_possible_moves = [&piece_possible_moves](CCheckersPlayer& player) -> move_container {
		auto& P = player.pieces;
		auto A = std::bind<move_container>(accumulete_moves<CCheckersPiece&>, std::placeholders::_1, std::placeholders::_2, std::cref(piece_possible_moves));
		return std::accumulate(std::begin(P), std::end(P), move_container(), A);
	};

	// 4. a. If no moves, player was defeated
	//    b. If one possible move, do it
	//    c. If multiply possible moves, ask behavior to decide and do it

	for ( ; ; )
	{
		auto moves = player_possible_moves(now_player);
		if (moves.empty())
			return next_player;

		CCheckersMovesIt mit = moves.cbegin();
		if (moves.size() > 1)
			mit = now_player.get().behavior.ChooseNextMove(*this, now_player, mit, moves.cend());

		//{
		//	auto& MOVS = mit->moves;
		//	auto str = std::accumulate(std::next(MOVS.begin()), MOVS.end(), CheckersHelpers::PosDesc(MOVS.front().get().Pos()),
		//		[](std::string acc, const CCheckersTile& tile) { acc.push_back('-'); acc.append(CheckersHelpers::PosDesc(tile.Pos())); return acc; });
		//	if (mit->Capture())
		//		str.push_back('$');
		//
		//	std::cout << str << std::endl;
		//}

		mit->Perform();

		std::swap(now, next);
	}
}

CCheckers::tilesit CCheckers::tbegin() const
{
	return board.cbegin();
}

CCheckers::tilesit CCheckers::tend() const
{
	return board.cend();
}

void CCheckers::Init(std::initializer_list<std::reference_wrapper<CCheckersTile>> whites, std::initializer_list<std::reference_wrapper<CCheckersTile>> blacks)
{
	std::for_each(std::begin(whites), std::end(whites), std::bind(&CCheckersPlayer::Piece, std::ref(white), std::placeholders::_1));
	std::for_each(std::begin(blacks), std::end(blacks), std::bind(&CCheckersPlayer::Piece, std::ref(black), std::placeholders::_1));
}

CheckersPlayerColor CCheckers::WhitePlayer::Color() const
{
	return CheckersPlayerColor::White;
}

CheckersPlayerColor CCheckers::BlackPlayer::Color() const
{
	return CheckersPlayerColor::Black;
}

void CCheckers::print() const
{
	//     a b c d e f g h     
	//   -------------------   
	// 8 | o x o x o x o x | 8
	// 7 | x o x o x o x o | 7
	// 6 | o x o x o x o x | 6
	// 5 | x o x o x o x o | 5
	// 4 | o x o x o x o x | 4
	// 3 | x o x o x o x o | 3
	// 2 | o x o x o x o x | 2
	// 1 | x o x o x o x o | 1
	//   -------------------
	//     a b c d e f g h

	auto p_char = [] {
		std::cout << "     a b c d e f g h     " << std::endl;
	};
	auto p_line = [] {
		std::cout << "   -------------------   " << std::endl;
	};
	auto t = [](const CCheckersTile* t) {
		std::string s;
		auto c = { 'w', 'b' };
		if (t->Empty())
			s = "x";
		else
			s = *(c.begin() + static_cast<size_t>(t->Piece().value().get().Color()));
		return s;
	};
	auto p_board = [&t] (size_t n, std::initializer_list<const CCheckersTile*> &&l) {
		std::array<std::string, 4> q;
		std::transform(l.begin(), l.end(), q.begin(), t);
		std::cout << " " << n << " | o " << q[0] << " o " << q[1] << " o " << q[2] << " o " << q[2] << " | " << n << " " << std::endl;
	};
	auto p_board_odd = [&t](size_t n, std::initializer_list<const CCheckersTile*>&& l) {
		std::array<std::string, 4> q;
		std::transform(l.begin(), l.end(), q.begin(), t);
		std::cout << " " << n << " | " << q[0] << " o " << q[1] << " o " << q[2] << " o " << q[3] << " o | " << n << " " << std::endl;
	};

	std::cout << std::endl;
	p_char();
	p_line();
	p_board(8, { &B8, &D8, &F8, &H8 });
	p_board_odd(7, { &A7, &C7, &E7, &G7 });
	p_board(6, { &B6, &D6, &F6, &H6 });
	p_board_odd(5, { &A5, &C5, &E5, &G5 });
	p_board(4, { &B4, &D4, &F4, &H4 });
	p_board_odd(3, { &A3, &C3, &E3, &G3 });
	p_board(2, { &B2, &D2, &F2, &H2 });
	p_board_odd(1, { &A1, &C1, &E1, &G1 });
	p_line();
	p_char();

	
}
