# -*- coding: utf-8 -*-
"""
Python 2.7
Testa a logica de escritores e leitores do arquivo log gerado pelo programa em C
"""
#Listas que indicam quantos leitores ou escritoras estao em executacao  
leitoras = []
escritoras = []

# Sinalizadores para l(leitor) e s(escritor). Eles indicam que tenho as condicoes necessarias para le ou escrever
l = 0 
s = 0

def entra_leitora_primeira():
    global l, leitoras
    n = len(leitoras)        # Quantidade de leitoras na lista 
    if ( n == 0 and l == 0): # Se lista de leitoras vazias e variavel de condicao l = 0
        l =  1               # Variavel l vai para 1 e permite leitoras e "bloqueia" escritores
    else:
        print "Erro na entrada da primeira leitura, não atendeu as condicoes necessarias"
        quit()

def entra_leitora():
    global l, s, leitoras
    if (l == 1 and s == 0): # Se s = 0 indica que não tenho escrita sendo realizadas
        leitoras.append(1)  # Insere 1 na lista leitora, indicando um leitor lendo
        print "leitoras: "
        print leitoras      # Printa lista de leitoras naquele momento
    else:
        print "Erro na leitura: ainda pode haver escritoras em execucao"
        quit()

def sai_leitora():
    global l, s, leitoras
    if (l == 1 and s == 0): # Mesma condicao do entre_leitora().
        del(leitoras[0])    # Retira 1 da lista leitora, indicando o termino de leitor
        print "leitoras: "
        print leitoras      # Printa lista de leitoras naquele momento  
    else:
        print "Erro na saida da leitura"
        quit()
    
def sai_leitora_ultima():
    global l, leitoras
    n = len(leitoras)       # Pega a quantidade de leitoras na lista
    if ( n == 0 and l == 1):# Se todas as leitoras terminaram de le 
        l =  0              # Indico que escritoras podem excutar e novas leitoras no entra_primeira_leitora()
    else:
        print "Erro na saida da ultima leitura: ainda pode haver leitoras na fila"
        quit()

def entra_escrita():        
    global l, s, escritoras
    n = len(escritoras)     # Pega a quantidade de escritoras na fila
    if (n == 0 and s == 0): # Condicao sera aceita se nao tiver escritoras em execucao 
        s =  1              # Sinalizo que tem escrita em execucao
        if (s == 1 and l == 0 and n < 1):  
            escritoras.append(1)    # Insere 1 na lista de escritores
            print "Escritoras: "
            print escritoras        # Printa lista de escritores no momento 
        else:
            print "Erro na escrita: Pode haver outra escrita em execucao ou leitora"
            quit()
    else:
        print "Erro na escrita: condicoes nao atendidas"
        quit()
                  
            
def sai_escrita():
    global l, s, escritoras 
    if (s == 1 and l == 0): # Se condicao para sair da escrita atendidas
        del(escritoras[0])  # Retira a escritora da lista
        print "Escritoras: "
        print escritoras    # Printa lista de escritores no momento
        n = len(escritoras) # Pega a quantidade de escritoras na fila
        if ( n == 0 and s == 1): # Se quantidade n = 0 e condicao de escrita em execucao
            s =  0          # Indico que nao ha mais escritas, permitindo novas escritas ou leituras
        else:
            print "Erro na escrita"
            quit()
    else:
        print "Erro saida da escrita"
        quit()

# Fucao inserida no final do arquivo de log pela da execucao do programa em C
def teste_final():
    global leitoras, escritoras
    t1 = len(leitoras)          # Pego a quantidade de leitoras
    t2 = len(escritoras)        # Pego a quantidade de escritoras
    if ( t1 == 0 and t2 == 0):  # Se as duas terminaram vazias
        print "Ok!"             # Condição final aceita
    else:
        print "Erro no final!"
   
    
#Testa o programa aqui simulando os log  
def main():
    entra_leitora_primeira() # Necessariamente uma entra_leitora_primeira() vem seguida de uma entra_leitora(), segundo o programa que gera os logs.
    entra_leitora()
    entra_leitora()
    sai_leitora ()
    entra_leitora()
    sai_leitora ()
    sai_leitora ()
    sai_leitora_ultima()
    entra_escrita()
    sai_escrita()
    teste_final()
# Descomentar aqui em baixo para executar os testes da main()
#main()
