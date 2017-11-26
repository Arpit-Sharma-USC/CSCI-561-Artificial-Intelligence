import time
import copy


class Stack():
    def __init__(self):
        self.stack = []

    def isEmpty(self):
        if len(self.stack) == 0:
            return True
        return False

    def get(self):
        if not self.isEmpty():
            first = self.stack.pop(0)
            return first
        return False

    def add(self, value):
        self.stack.insert(0, value)


MOVES = None
TIME_UP = False
IS_MOVES_SET = False
TIME_ON_A_MOVE = None


class Board_State():
    def __init__(self):
        self.value = None
        self.score = 0
        self.depth = 0
        self.matrix = None
        self.move = None
        self.remove_list = []


def getTempMatrix(fruit_box):
    fruit_box = copy.deepcopy(fruit_box)
    return fruit_box

def console_check(state):
    global TIME_UP
    matrix = state.matrix
    sTemp = set()
    for row in matrix:
        sTemp.update(set(row))

    sTemp = list(sTemp)
    if len(sTemp) == 1 and sTemp[0] == '*':
        return True
    if state.depth == MAX_DEPTH:
        return True
    if IS_MOVES_SET and time.time() - start_time > TIME_ON_A_MOVE:
        TIME_UP = True
        return True
    return False


def gravity(fruit_box):
    for j in range(sizeN):
        mPrime = sizeN - 1
        nPrime = sizeN - 1
        while nPrime > -1:
            if fruit_box[nPrime][j] != '*' and mPrime == nPrime:
                nPrime = nPrime- 1
                mPrime = mPrime- 1
            elif fruit_box[nPrime][j] == '*':
                nPrime = nPrime- 1
            elif fruit_box[nPrime][j] != '*' and nPrime < mPrime:
                fruit_box[mPrime][j] = fruit_box[nPrime][j]
                nPrime = nPrime- 1
                mPrime = mPrime- 1

        if mPrime > nPrime:
            while mPrime >= 0:
                fruit_box[mPrime][j] = '*'
                mPrime = mPrime- 1

    return fruit_box


def searchDfs(fruit_box, i, j, new_depth, total_score, flag):
    stack = Stack()
    stack.add((i, j))

    score = 1

    value = fruit_box[i][j]
    fruit_box[i][j] = '*'
    remove_list = []
    while not stack.isEmpty():
        x, y = stack.get()
        xPrime, yPrime = x, y + 1
        if yPrime < sizeN and fruit_box[xPrime][yPrime] == value:
            stack.add((xPrime, yPrime))
            fruit_box[xPrime][yPrime] = '*'
            remove_list.append((xPrime, yPrime))
            score = score + 1

        xPrime, yPrime = x - 1, y
        if xPrime >= 0 and fruit_box[xPrime][yPrime] == value:
            stack.add((xPrime, yPrime))
            fruit_box[xPrime][yPrime] = '*'
            remove_list.append((xPrime, yPrime))
            score = score + 1

        xPrime, yPrime = x + 1, y
        if xPrime < sizeN and fruit_box[xPrime][yPrime] == value:
            stack.add((xPrime, yPrime))
            fruit_box[xPrime][yPrime] = '*'
            remove_list.append((xPrime, yPrime))
            score = score + 1

        xPrime, yPrime = x, y - 1
        if yPrime >= 0 and fruit_box[xPrime][yPrime] == value:
            stack.add((xPrime, yPrime))
            fruit_box[xPrime][yPrime] = '*'
            remove_list.append((xPrime, yPrime))
            score = score + 1

    state = Board_State()
    state.value = score ** 2
    state.score = total_score + flag * (score ** 2)
    state.move = (i, j)
    state.matrix = gravity(fruit_box)
    state.depth = new_depth
    state.remove_list = remove_list
    return state


def actions(state, flag):
    global count
    global MOVES
    global IS_MOVES_SET
    global TIME_ON_A_MOVE

    matrix = state.matrix
    list_of_possible_actions = []
    list_of_exhausted_positions = []
    for j in range(sizeN):
        for i in range(sizeN - 1, -1, -1):
            if matrix[i][j] == '*':
                break
            if ((i, j) not in list_of_exhausted_positions):
                new_matrix = getTempMatrix(matrix)
                new_state = searchDfs(new_matrix, i, j, state.depth + 1, state.score, flag)
                list_of_possible_actions.append(new_state)
                list_of_exhausted_positions = list_of_exhausted_positions + new_state.remove_list

    list_of_possible_actions = sorted(list_of_possible_actions, key=lambda temp_x: temp_x.value, reverse=True)

    if not IS_MOVES_SET:
        MOVES = len(list_of_possible_actions)
        IS_MOVES_SET = True

        if MOVES > 4:
            TIME_ON_A_MOVE = (2.0 * float(TIME) * 0.8) / float(MOVES)
        else:
            TIME_ON_A_MOVE = (2.0 * float(TIME) * 0.9) / float(MOVES)
        #time.sleep(.010)
        #print TIME_ON_A_MOVE #can comment this and save terminal output space
        set_max_depth(init=0)
    return list_of_possible_actions


def alphaBetaSearch(state):
    state, move = maxValFunc(state, -99999999, 999999999)
    state.move = move
    return state


def value_setter(state, function_type):
    if TIME_UP:
        if function_type == "max":
            state.value = float(-999999999)
        else:
            state.value = float(999999999)
    else:
        score = state.score
        state.value = float(score)
    return state


def minValFunc(state, alpha, beta):
    if console_check(state):
        set_value = value_setter(state, "min")
        return set_value, set_value.move

    v = 99999999
    move = None
    for a_state in actions(state, -1):
        maxValuePrime, garbage_no_use = maxValFunc(a_state, alpha, beta)

        if v > maxValuePrime.value:
            move = maxValuePrime.move
            v = maxValuePrime.value
            state.value = maxValuePrime.value
            state.matrix = maxValuePrime.matrix
            state.score = maxValuePrime.score

        if v <= alpha:
            return state, move
        beta = min(beta, v)
    return state, move


def maxValFunc(state, alpha, beta):
    if console_check(state):
        value_set = value_setter(state, "max")
        return value_set, value_set.move

    v = -99999999
    move = None
    for a_state in actions(state, 1):
        minValuePrime, garbage_no_use = minValFunc(a_state, alpha, beta)

        if v < minValuePrime.value:
            move = minValuePrime.move
            v = minValuePrime.value
            state.value = minValuePrime.value
            state.matrix = minValuePrime.matrix
            state.score = minValuePrime.score

        if v >= beta:
            return state, move
        alpha = max(alpha, v)
    return state, move



def set_max_depth(init=1):
    global MAX_DEPTH
    global sizeN

    if init == 1:
        if sizeN < 8:
            MAX_DEPTH = 5
        elif sizeN > 7 and sizeN < 14:
            MAX_DEPTH = 4
        else:
            MAX_DEPTH = 3

    if init == 0 and sizeN > 13:
        if IS_MOVES_SET and MOVES > 41:
            MAX_DEPTH = 3
        elif MOVES <= 41 and MOVES > 20:
            MAX_DEPTH = 4
        else:
            MAX_DEPTH = 5


def main():
    global sizeN
    global start_time
    global MOVES
    global IS_MOVES_SET
    global TIME_ON_A_MOVE
    global TIME_UP
    global TIME
    global count


    MOVES = None
    IS_MOVES_SET = False
    TIME_ON_A_MOVE = None
    TIME_UP = False

    start_time = time.time()

    with open("input.txt", "r") as f:

        inp = f.read().strip().split("\n")
        TIME = float(inp[2])
        sizeN = int(inp[0])
        P = int(inp[1])
        set_max_depth(init=1)

        matrix = []
        m = inp[3:]

        i = 0
        j = 0
        for i in range(sizeN):
            temp_list = []
            for j in range(sizeN):
                temp_list.append(m[i][j])
            matrix.append(temp_list)

        init_matrix = copy.deepcopy(matrix)
        state = Board_State()
        state.value = 0
        state.score = 0
        state.matrix = matrix
        state.depth = 0
        state.move = None

        alphaBetaSearchTemp = alphaBetaSearch(state)

        temp = searchDfs(init_matrix, alphaBetaSearchTemp.move[0], alphaBetaSearchTemp.move[1], 0, 0, 0)
        alphaBetaSearchTemp.matrix = temp.matrix
        alphaBetaSearchTemp.score = temp.value

        fp = open("output.txt", "w")
        fp.write(chr(alphaBetaSearchTemp.move[1] + 65) + str(alphaBetaSearchTemp.move[0] + 1) + "\n")

        matrix = alphaBetaSearchTemp.matrix
        for i in range(sizeN):
            fp.write("".join(matrix[i]) + "\n")

        fp.close()


if __name__ == "__main__":
    main()
