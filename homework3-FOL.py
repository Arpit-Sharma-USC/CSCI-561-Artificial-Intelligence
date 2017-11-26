import random
import sys
import os
import numpy as np
import copy
import time
#Author: Arpit Sharma#

class rule:
    sentence = None
    def __init__(self):
        sentence = []

class predicate:
    name = None
    sign = None
    constant = None
    variable = None
    arg_list = None

    def __init__(self):
        self.arg_list = []
        self.name = ""
        self.sign = 0
        self.constant = []
        self.variable = []

###################################


def predicate_parsing(input_stream):
    predicate_extracted = predicate()
    filtered_first_ltr = input_stream.split('(')
    temp_predicate = filtered_first_ltr[0]
    if(temp_predicate[0] == '~'):
        predicate_extracted.sign = 1
        temp_predicate=temp_predicate.replace("~","")
    predicate_extracted.name = temp_predicate
    temp_predicate = filtered_first_ltr[1].split(',')
    for i in range(temp_predicate.__len__()):
        len = temp_predicate[i].__len__()
        if(temp_predicate[i].find(")")> -1):
            sign = temp_predicate[i].find(")")
            temp_predicate[i] = temp_predicate[i][:sign]
        if(temp_predicate[i][0].isupper()):
            predicate_extracted.constant.append(temp_predicate[i])
            predicate_extracted.arg_list.append(temp_predicate[i])
        else:
            predicate_extracted.variable.append(temp_predicate[i])
            predicate_extracted.arg_list.append(temp_predicate[i])
    return predicate_extracted

def copySentence(rule):
    newRule = []
    editedPredicate = predicate()

    for i in range(rule.__len__()):#changes here
        previousPredicate = rule[i]
        for j in range(previousPredicate.arg_list.__len__()):#and here
            editedPredicate.arg_list.append(previousPredicate.arg_list[j])
        editedPredicate.sign = previousPredicate.sign
        editedPredicate.name = previousPredicate.name
        newRule.append(editedPredicate)
        editedPredicate = predicate()

    return newRule

def unify(Rule1, Rule2, predicate1, predicate2, index1, index2):

    sentence1 = []
    sentence2 = []

    sentence1 = copySentence(Rule1)
    sentence2 = copySentence(Rule2)

    constant_variable_collection = []
    const_var = []

    predicate_already_unified = predicate1
    predicate_to_unify = predicate2

    unificationFlag=False


    for i in range(predicate_already_unified.arg_list.__len__()):
        isPredicate1ArgumentConstant = predicate_already_unified.arg_list[i][0].isupper()
        isPredicate2ArgumentConstant = predicate_to_unify.arg_list[i][0].isupper()
        if(isPredicate1ArgumentConstant == True and isPredicate2ArgumentConstant == False):
            const_var.append(predicate_already_unified.arg_list[i])
            const_var.append(predicate_to_unify.arg_list[i])
            constant_variable_collection.append(const_var)
            const_var = []

    for i in range(predicate_already_unified.arg_list.__len__()):
        isPredicate1ArgumentConstant = predicate_already_unified.arg_list[i][0].isupper()
        isPredicate2ArgumentConstant = predicate_to_unify.arg_list[i][0].isupper()
        if (isPredicate1ArgumentConstant == True and isPredicate2ArgumentConstant == True):
            if(predicate_already_unified.arg_list[i]!=predicate_to_unify.arg_list[i]):
                return None

    for i in range(sentence2.__len__()):
        for j in range(sentence2[i].arg_list.__len__()):
            for k in range(constant_variable_collection.__len__()):
                if(sentence2[i].arg_list[j] == constant_variable_collection[k][1]):
                    sentence2[i].arg_list[j] = constant_variable_collection[k][0]

    constant_variable_collection = []


    for i in range(sentence1.__len__()):
        for j in range(sentence1[i].arg_list.__len__()):
            for k in range(constant_variable_collection.__len__()):
                if (sentence1[i].arg_list[j] == constant_variable_collection[k][1]):
                    sentence1[i].arg_list[j] = constant_variable_collection[k][0]

    for i in range(predicate_already_unified.arg_list.__len__()):
        isPredicate1ArgumentConstant = predicate_already_unified.arg_list[i][0].isupper()
        isPredicate2ArgumentConstant = predicate_to_unify.arg_list[i][0].isupper()
        if(isPredicate1ArgumentConstant == False and isPredicate2ArgumentConstant == True):
            const_var.append(predicate_to_unify.arg_list[i])
            const_var.append(predicate_already_unified.arg_list[i])
            constant_variable_collection.append(const_var)


    for index in range(predicate_already_unified.arg_list.__len__()):
        isPredicate1ArgumentConstant = predicate_already_unified.arg_list[index][0].isupper()
        isPredicate2ArgumentConstant = predicate_to_unify.arg_list[index][0].isupper()
        if(isPredicate1ArgumentConstant == False and isPredicate2ArgumentConstant == False):
            sentence1Variable = predicate_already_unified.arg_list[index]
            sentence2Variable = predicate_to_unify.arg_list[index]

            if(sentence1Variable != sentence2Variable):
                for i in range(sentence2.__len__()):
                    for j in  range(sentence2[i].arg_list.__len__()):
                        if(sentence2[i].arg_list[j] == sentence1Variable):
                            sentence2[i].arg_list[j] += '7'

                for i in range(sentence2.__len__()):
                    for j in  range(sentence2[i].arg_list.__len__()):
                        if(sentence2[i].arg_list[j] == sentence2Variable):
                            sentence2[i].arg_list[j] = sentence1Variable

            const_var.append(predicate_to_unify.arg_list[index])
            const_var.append(predicate_already_unified.arg_list[index])
            constant_variable_collection.append(const_var)

    del sentence2[index2]
    del sentence1[index1]

    resultantSentence = sentence1 + sentence2

    if(check_for_repeated_sentences(resultantSentence)):
        return None
    return resultantSentence

################function to check if the sentence is being used again###########
def duplicate_deducer(first_sentence, second_sentence, suspectedIndex, similar):
    if(suspectedIndex <first_sentence.__len__()):
        first_pred = first_sentence[suspectedIndex]
        second_pred = second_sentence[suspectedIndex]
        if(first_pred.name != second_pred.name):
            return False
        for i in range(0, first_sentence[suspectedIndex].arg_list.__len__()):
            tempA = first_pred.arg_list[i]
            tempB = second_pred.arg_list[i]
            if(tempA[0].isupper() and tempB[0].isupper and tempA!=tempB):
                return False
            for j in range(0, similar.__len__()):
                pehla = (similar[j][0] == tempA)
                doosra = (similar[j][1] == tempB)
                if(pehla != doosra ):
                    return False
            similar.append((tempA, tempB))
        return duplicate_deducer(first_sentence, second_sentence, suspectedIndex + 1, similar)
    else:
        return True
####end of duplicate deducer######


######main contradiction checking step in the resolution procedure which tells the resolve_crux when to stop when to stop############
def contradiction_checking_agent(first_predicate, second_predicate):
    if(first_predicate.name == second_predicate.name and first_predicate.sign != second_predicate.sign):
        if(first_predicate.arg_list.__len__() == second_predicate.arg_list.__len__()):
            for iPrime in range(first_predicate.arg_list.__len__()):
                if(first_predicate.arg_list[iPrime] != second_predicate.arg_list[iPrime]):
                    return False
            return True
    return False
####end of check agent#####


#####crux of the code, checks for predicates in all sentences in the K_Base with opposite sign(~) and same name as query_sent###
def resolve_crux(K_Base, querySent, recursion_threshold = 700):
    recursion_threshold-=1
    if(recursion_threshold ==0):
        return 'recursion_threshold_breached'
    if(recursion_threshold==690):
        x=0
    for i in range(K_Base.__len__()):
        current_sentence = K_Base[i].copy()
        for j in range(current_sentence.__len__()):
            current_predicate = current_sentence[j]
            for k in range(querySent.__len__()):
                queryDaPredicate = querySent[k]
                if (current_predicate.name == queryDaPredicate.name and current_predicate.sign != queryDaPredicate.sign):
                    if (querySent.__len__() == 1 and current_sentence.__len__() == 1):
                        if (contradiction_checking_agent(querySent[0], current_sentence[0])):
                            return 'true'
                    sentenceAfterUnification = unify(querySent, current_sentence.copy(), queryDaPredicate,
                                            current_predicate, k, j)

                    if(sentenceAfterUnification != None):
                        if (sentenceAfterUnification.__len__() == 0):
                            return 'true'
                        if(loop_detection(sentenceAfterUnification)==False):
                            listOfDuplicatePredicates.append(sentenceAfterUnification)
                            result = resolve_crux(K_Base, sentenceAfterUnification, recursion_threshold)
                            if(result == 'recursion_threshold_breached' or result == 'true'):
                                return result
                        else:
                            continue
###end of resolve crux###

#### function to check if there is a loop of same predicates that exists; calls duplicate_deducer#####
def loop_detection(sentenceAfterUnification):
    if(listOfDuplicatePredicates.__len__()==0):
        return False

    flag = False

    for sentence in listOfDuplicatePredicates:
        if(sentence.__len__() == sentenceAfterUnification.__len__()):
            flag = duplicate_deducer(sentence, sentenceAfterUnification, 0, [])
            if(flag):
                return flag

    return flag
#######end of loop detection########

####checking for repeated sentences ###
def check_for_repeated_sentences(sentenceAfterUnification):
    for i in range(sentenceAfterUnification.__len__()):
        for j in range(sentenceAfterUnification.__len__()):
            if (i == j):
                continue
            if (sentenceAfterUnification[i].sign == sentenceAfterUnification[j].sign):
                if (sentenceAfterUnification[i].name == sentenceAfterUnification[j].name):
                    for k in range(sentenceAfterUnification[i].arg_list.__len__()):
                        if (sentenceAfterUnification[i].arg_list[k] != sentenceAfterUnification[j].arg_list[k]):
                            break
                    return True

    return False

#########end of check of duplicate sentences #############

####################################main function ##########################

listOfDuplicatePredicates =[]

istream = "input.txt"
input_stream = []

q_u_e_r_y_list = []

fp = open(istream, "r")

# istream data  retrieval
for i in fp:
    input_stream.append(i)

q_u_e_r_y_How_many = int(input_stream[0])
s_e_n_t_e_n_c_e_s_how_many = int(input_stream[q_u_e_r_y_How_many + 1])

q_u_e_r_y_list = []
rule = []
K_Base = []

for i in range(1, q_u_e_r_y_How_many+1):
    q_u_e_r_y_list.append(predicate_parsing(input_stream[i]))

for temp in range(q_u_e_r_y_How_many+2, s_e_n_t_e_n_c_e_s_how_many+q_u_e_r_y_How_many+2):
    if(input_stream[temp].find("|") > -1):
        input_stream[temp] = input_stream[temp].replace(" ","")
        orSplitPoint = input_stream[temp].split("|")
        for splitTemp in range(orSplitPoint.__len__()):
            rule.append(predicate_parsing(orSplitPoint[splitTemp]))
    else:
        rule.append(predicate_parsing(input_stream[temp]))
    K_Base.append(rule)
    rule = []

query_sent = []
query_sent.append(q_u_e_r_y_list[0])

canBeResolved = False

output_stream = open('output.txt', 'w')

for i in range(0, q_u_e_r_y_list.__len__()):
    query_sent=[]
    query_sent.append(q_u_e_r_y_list[i])
    query_sent[0].sign = abs(query_sent[0].sign - 1)
    listOfDuplicatePredicates = [query_sent]
    canBeResolved = resolve_crux(K_Base.copy(), query_sent)
    result = None
    if(not canBeResolved == "true"):
        result = "FALSE\n"
        #K_Base.append(query_sent)
    else:
        result = "TRUE\n"

    # write to output fp
    output_stream.write(result)
########end of main###########
##############end of code#################
