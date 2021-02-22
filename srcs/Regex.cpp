#include "Regex.hpp"

Regex::Regex(string const & regex) throw (std::invalid_argument) : _source(regex) {
	if (regex.size() == 0)
		throw std::invalid_argument("Regex can't be empty");
	else if (_isRealEscape(_source.size() - 1))
		throw std::invalid_argument("Regex Pattern may not end with a trailing backslash");
	else if (_source.size() > SOURCE_MAX_SIZE)
		throw std::invalid_argument("Regex Pattern is too long");
	_checkPipeValidity();
	_checkParenthesisValidity();
	_checkDelimiterValidity();
	_checkBracketValidity();
	size_t i = 0;
	while (_source[i])
		_createSequence(i, _root);
}

Regex::Regex( Regex const & o) :  _root(o._root), _source(o._source) {
}

Regex::~Regex() {
}

Regex & Regex::operator=(Regex const & o) {
	_source = o._source;
	_root = o._root;
	return (*this);
}

bool Regex::operator<(Regex const & o) const {
	return (_source.size() > o._source.size() ||
		(_source.size() == o._source.size() && _source > o._source));
}


/* Public */

string Regex::getSource() const {
	return (_source);
}

bool Regex::match(string const & str) const {
	size_t strPos = 0;
	size_t testedStrPos = 0;
	size_t limit = str.size();

	for (; strPos <= limit; ++strPos) {
		testedStrPos = strPos;
		if (_matchSequence(str, testedStrPos, _root.sequence, 0))
			return (true);
	}
	return (false);
}


/* Checks */

void Regex::_checkPipeValidity() const throw (std::invalid_argument) {
	if (_source[0] == '|' || _isRealPipe(_source.size() - 1))
		throw std::invalid_argument("Regex pipe error");
	size_t size = _source.size();
	for (size_t i = 1; i < size; ++i)
		if (_isRealPipe(i) && _isRealPipe(i - 1))
			throw std::invalid_argument("Regex pipe error");
}

void Regex::_checkParenthesisValidity() const throw (std::invalid_argument) {
	ssize_t parenthesisCount = 0;

	size_t size = _source.size();
	for (size_t i = 0; i < size; ++i) {
		if (_source[i] == '(' && !_isEscaped(i))
			++parenthesisCount;
		else if (_source[i] == ')' && !_isEscaped(i))
			--parenthesisCount;
		if (parenthesisCount < 0)
			throw std::invalid_argument("Regex parenthesis error");
	}
	if (parenthesisCount != 0)
		throw std::invalid_argument("Regex parenthesis error");
}

void Regex::_checkDelimiterValidity() const throw (std::invalid_argument) {
	for (size_t i = 0; _source[i]; ++i)
		if (!_isEscaped(i) && i && _source[i] == '^' && !_isInBracket(i))
			throw std::invalid_argument("Regex invalid ^ position");
		else if (!_isEscaped(i) && _source[i + 1] && _source[i] == '$')
			throw std::invalid_argument("Regex invalid $ position");
}

void Regex::_checkBracketValidity() const throw (std::invalid_argument) {
	ssize_t leftMinRangePos;

	for (ssize_t i = 0; _source[i]; ++i)
		if (_isRealOpeningBracket(i)) {
			size_t bracketEnd = _getBracketEnd(i);
			if (bracketEnd == string::npos)
				throw std::invalid_argument("Regex missing ]");
			else if (bracketEnd == static_cast<size_t>(i + 1))
				throw std::invalid_argument("Regex bracket can't be empty");
			++i;
			leftMinRangePos = i;
			for (; static_cast<size_t>(i) < bracketEnd; ++i) {
				if (i - 1 >= leftMinRangePos && _isInRange(i, 1, bracketEnd - 2) && _source[i] == '-' && !_isEscaped(i)) {
					if (_source[i - 1] > _source[i + 1])
						throw std::invalid_argument("Regex [] error min > max");
					++i; leftMinRangePos = i + 1;
				}
			}
		}
}


/* Parsing */

void Regex::_createSequence(size_t & i, struct pattern & parent)  throw (std::invalid_argument) {
	struct pattern sequence("Sequence");

	size_t sequenceEnd = _getSequenceEnd(i);
	while (i < sequenceEnd)
		_handleSequence(i, sequence, parent);
	_insertSequence(sequence, parent);
}

void Regex::_handleSequence(size_t & i, struct pattern & sequence, struct pattern & parent)  throw (std::invalid_argument) {
	if (_isQuantifier(i))
		throw std::invalid_argument("Regex the preceding token is not quantifiable");
	if (_isRealEscape(i))
		_handleEscapeCharacter(i);
	else if (_isRealOpeningParenthesis(i))
		_handleParenthesis(i, sequence);
	else if (_isRealOpeningBracket(i))
		_handleBracket(i, sequence);
	else if (_isRealPipe(i))
		_handlePipe(i, sequence, parent);
	else
		_handleCharacter(i , sequence);
}

void Regex::_handleParenthesis(size_t & i, struct pattern & sequence) throw (std::invalid_argument) {
	struct pattern parenthesis("Parenthesis");

	++i;
	size_t parenthesisEnd = _getParenthesisEnd(i);
	while (i < parenthesisEnd)
		_createSequence(i , parenthesis);
	++i;
	_setPatternMinMax(i, parenthesis);
	sequence.sequence.push_back(parenthesis);
}

void Regex::_handleBracket(size_t & i, struct pattern & sequence) throw (std::invalid_argument) {
	struct pattern bracket;

	size_t bracketEnd = _getBracketEnd(i);
	bracket.value = string(_source, i, bracketEnd - i + 1);
	i = bracketEnd + 1;
	_setPatternMinMax(i, bracket);
	sequence.sequence.push_back(bracket);
}

void Regex::_handlePipe(size_t & i, struct pattern & sequence, struct pattern & parent) throw (std::invalid_argument) {
	sequence.value = "Pipe";
	parent.isAlternative = true;
	++i;
	size_t pipeEnd = _getPipeEnd(i);
	while (i < pipeEnd)
		_handleSequence(i, sequence, parent);
}

void Regex::_handleCharacter(size_t & i, struct pattern & sequence) {
	size_t characterEnd = _getCharacterEnd(i);
	while (i < characterEnd) {
		if (_isRealEscape(i)) {
			++i;
			continue ;
		}
		struct pattern actualChar;
		actualChar.value = _source[i];
		actualChar.isEscaped = _isEscaped(i);
		++i;
		_setPatternMinMax(i, actualChar);
		sequence.sequence.push_back(actualChar);
	}
}

void Regex::_handleEscapeCharacter(size_t & i) const {
	++i;
}

void Regex::_setPatternMinMax(size_t & i, struct pattern & p) throw (std::invalid_argument) {
	if (_source[i] == '*' && !_isEscaped(i))
		{p.min = 0; p.max = UNLIMITED; ++i;}
	else if (_source[i] == '+' && !_isEscaped(i))
		{p.min = 1; p.max = UNLIMITED; ++i;}
	else if (_source[i] == '?' && !_isEscaped(i))
		{p.min = 0; p.max = 1; ++i;}
	else if (_isRangeQuantifier(i))
		_setRangeQuantifier(i, p);
}

void Regex::_insertSequence(struct pattern & sequence, struct pattern & parent) {
	if (parent.isAlternative)
		parent.sequence.back().alternative.push_back(sequence);
	else
		parent.sequence.push_back(sequence);
}

size_t Regex::_getParenthesisEnd(size_t i) const {
	size_t end = i;
	ssize_t parenthesisCount = 1;

	while(_source[end]) {
		if (_isRealOpeningParenthesis(end))
			++parenthesisCount;
		else if (_isRealClosingParenthesis(end))
			--parenthesisCount;
		if (parenthesisCount == 0)
			break ;
		++end;
	}
	return (end);
}

size_t Regex::_getBracketEnd(size_t i) const {
	while(_source[i] && !_isRealClosingBracket(i))
		++i;
	return (_source[i] ? i : string::npos);
}

size_t Regex::_getPipeEnd(size_t i) const {
	while(_source[i]) {
		if (_isRealPipe(i) || _isRealClosingParenthesis(i))
			return (i);
		if (_isRealOpeningParenthesis(i))
			i = _getParenthesisEnd(i);
		else
			++i;
	}
	return (i);
}

size_t Regex::_getCharacterEnd(size_t i) const {
	while(_source[++i] && !_isRealPipe(i) && !_isRealOpeningParenthesis(i)
	&& !_isRealClosingParenthesis(i) && !_isRealOpeningBracket(i));
	return (i);
}

size_t Regex::_getSequenceEnd(size_t i) const {
	ssize_t openedParenthesis = 0;
	if (_isRealPipe(i))
		++i;
	for (; _source[i]; ++i) {
		if (_isRealOpeningParenthesis(i))
			++openedParenthesis;
		else if (_isRealClosingParenthesis(i))
			--openedParenthesis;
		if (openedParenthesis < 0 || (_isRealPipe(i) && openedParenthesis == 0))
			break;
	}
	return (i);
}


/* Match */

bool Regex::_matchSequence(string const & str, size_t & strPos, vector<struct pattern> const & sequence, size_t sequencePos) const {
	size_t alternativeSize, iAlternative, strPosSave = strPos;

	if (sequencePos == sequence.size())
		return (true);
	if (sequence[sequencePos].min == 0
	&& _matchSequence(str, strPos, sequence, sequencePos + 1) == true)
		return (true);

	for (size_t repeat = 1; repeat <= sequence[sequencePos].max && repeat <= str.size(); ++repeat) {
		strPos = strPosSave;
		if (_matchPattern(str, strPos, sequence[sequencePos])) {
			strPosSave = strPos;
			if (repeat >= sequence[sequencePos].min && _matchSequence(str, strPos, sequence, sequencePos + 1) == true)
				return (true);
		}
		else {
			iAlternative = 0;
			alternativeSize = sequence[sequencePos].alternative.size();
			for (; iAlternative < alternativeSize && !_matchPattern(str, strPos, sequence[sequencePos].alternative[iAlternative]); ++iAlternative);
			if (iAlternative < alternativeSize) {
				strPosSave = strPos;
				if (repeat >= sequence[sequencePos].min
				&& _matchSequence(str, strPos, sequence, sequencePos + 1) == true)
					return (true);
			}
			else
				return (false);
		}
	}
	return (false);
}

bool Regex::_matchPattern(string const & str, size_t & strPos, struct pattern const & pattern) const {
	if (pattern.sequence.size() != 0)
		return (_matchSequence(str, strPos, pattern.sequence, 0));

	if (pattern.isEscaped)
		return (_matchCharacter(str, strPos, pattern));
	else if (pattern.value == ENDOFLINE)
		return (!str[strPos]);
	else if (!str[strPos])
		return(false);
	else if (pattern.value == ".")
		return (_matchDot(str, strPos));
	else if (pattern.value == STARTOFLINE)
		return (!strPos);
	else if (pattern.value[0] == '[')
		return (_matchBracket(str, strPos, pattern));
	else
		return (_matchCharacter(str, strPos, pattern));
}

bool Regex::_matchDot(string const & str, size_t & strPos) const {
		if (str[strPos] != '\n') {
			++strPos;
			return (true);
		}
		return (false);
}

bool Regex::_matchCharacter(string const & str, size_t & strPos, struct pattern const & pattern) const {
	if (str[strPos] == pattern.value[0]) {
		++strPos;
		return (true);
	}
	return (false);
}

bool Regex::_matchBracket(string const & str, size_t & strPos, struct pattern const & pattern) const {
	if (pattern.value[1] == '^')
		return(_matchOutBracket(str, strPos, pattern.value.substr(2, pattern.value.size() - 3)));
	else
		return(_matchInBracket(str, strPos, pattern.value.substr(1, pattern.value.size() - 2)));
}

bool Regex::_matchInBracket(string const & str, size_t & strPos, string const & bracket) const {
	ssize_t size = bracket.size();
	ssize_t leftMinRangePos = 0;

	for (ssize_t i = 0; bracket[i]; ++i)
		if (i - 1 >= leftMinRangePos && _isInRange(i, 1, size - 2) && bracket[i] == '-' && !_isEscaped(i, bracket)) {
			if (_isInRange(str[strPos], bracket[i - 1], bracket[i + 1])) {
				++strPos;
				return (true);
			}
			++i; leftMinRangePos = i + 1;
		}
		else if (bracket[i] == str[strPos]) {
			++strPos;
			return (true);
		}
	return (false);
}

bool Regex::_matchOutBracket(string const & str, size_t & strPos, string const & bracket) const {
	ssize_t size = bracket.size();
	ssize_t leftMinRangePos = 0;

	for (ssize_t i = 0; bracket[i]; ++i)
		if (i - 1 >= leftMinRangePos && _isInRange(i, 1, size - 2) && bracket[i] == '-' && !_isEscaped(i, bracket)) {
			if (_isInRange(str[strPos], bracket[i - 1], bracket[i + 1]))
				return (false);
			++i; leftMinRangePos = i + 1;
		}
		else if (bracket[i] == str[strPos])
			return (false);
	++strPos;
	return (true);
}



/* Utils */

bool Regex::_isInRange(ssize_t value, ssize_t min, ssize_t max) const {
	return (value >= min && value <= max);
}

bool Regex::_isEscaped(ssize_t i) const {
	size_t count = 0;
	while (--i >= 0 && _source[i] == '\\')
		++count;
	return (count % 2);
}

bool Regex::_isEscaped(ssize_t i, string const & str) const {
	size_t count = 0;
	while (--i >= 0 && str[i] == '\\')
		++count;
	return (count % 2);
}

bool Regex::_isRealOpeningParenthesis(size_t i) const {
	return (_source[i] == '(' && !_isEscaped(i));
}

bool Regex::_isRealClosingParenthesis(size_t i) const {
	return (_source[i] == ')' && !_isEscaped(i));
}

bool Regex::_isRealOpeningBracket(size_t i) const {
	return (_source[i] == '[' && !_isEscaped(i));
}

bool Regex::_isRealClosingBracket(size_t i) const {
	return (_source[i] == ']' && !_isEscaped(i));
}

bool Regex::_isRealPipe(size_t i) const {
	return (_source[i] == '|' && !_isEscaped(i));
}

bool Regex::_isRealEscape(size_t i) const {
	return (_source[i] == '\\' && !_isEscaped(i));
}

bool Regex::_isQuantifier(size_t i) const throw(std::invalid_argument) {
	if (_isEscaped(i))
		return (false);
	if (_source[i] == '*' || _source[i] == '?' || _source[i] == '+')
		return (true);
	return (_isRangeQuantifier(i));
}

bool Regex::_isRangeQuantifier(size_t i) const throw(std::invalid_argument) {
	if (_isEscaped(i))
		return (false);
	if (_source[i] == '{') {
		std::istringstream ss(_source.substr(i + 1));
		ssize_t min = -1, max = -1; char c = 0;
		ss >> std::noskipws >> min >> c;
		if (!ss.good() || min < 0)
			return (false);
		if (c == '}') {
			if (min > USHRT_MAX)
				throw std::invalid_argument("Regex quantifier range is too large");
			return (true);
		}
		if (c == ',') {
			if (ss.peek() == '}') {
				if (min > USHRT_MAX)
					throw std::invalid_argument("Regex quantifier range is too large");
				return (true);
			}
			ss >> std::noskipws >> max;
			if (ss.good() && ss.peek() == '}') {
				if (min > USHRT_MAX || max > USHRT_MAX)
					throw std::invalid_argument("Regex quantifier range is too large");
				if (max < 0)
					return (false);
				if (max < min)
					throw std::invalid_argument("Regex the quantifier range is out of order");
				return (true);
			}
		}
	}
	return (false);
}

void Regex::_setRangeQuantifier(size_t & i, struct pattern & p) const {
	std::istringstream ss(_source.substr(i + 1));
	char c = 0;
	ss >> std::noskipws >> p.min >> c;
	if (c == '}')
		p.max = p.min;
	else if (ss.peek() == '}')
		p.max = UNLIMITED;
	else
		ss >> std::noskipws >> p.max;
	i = _source.find('}', i + 1) + 1;
}

bool Regex::_isDigit(size_t i) const {
	return (_source[i] >= '0' && _source[i] <= '9');
}

bool Regex::_isInBracket(size_t i) const {
	int l, r;
	for (l = i - 1; l >= 0 && !_isRealOpeningBracket(l); --l);
	for (r = i + 1; _source[r] && !_isRealClosingBracket(r); ++r);
	return (l >= 0 && _source[r]);
}


/* Debug */

void Regex::_showPattern(vector<struct pattern> & p, int x, bool isAlternative) {
	for (size_t i = 0; i < p.size(); ++i)
	{
		if (isAlternative)
			std::cout << "IS ALTERNATIVE " << std::endl;
		std::cout << "prof = " << x << " actual sequence i = " << i;
		std::cout << ", pattern value = " << p[i].value << " min = " << p[i].min << " max = " << p[i].max << std::endl;
		if (p[i].sequence.size())
		{
			std::cout << "rest of sequence:" << std::endl;
			_showPattern(p[i].sequence, x + 1);
		}
		if (p[i].alternative.size())
		{
			std::cout << "alternative sequence:" << std::endl;
			_showPattern(p[i].alternative, x + 1, true);
		}
	}
}
