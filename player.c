int main(){
    char input[30], command[10];
    
    for(){
        input = fgets(input, 30, stdin);
        command = strtok(input, " \t\n");
        if(strcmp(command, "start") || strcmp(command, "sg") == 0){
            start(input);
        }
        if(strcmp(command, "play") || strcmp(command, "pl") == 0){}
        if(strcmp(command, "guess") || strcmp(command, "gw") == 0){}
        if(strcmp(command, "scorebord") || strcmp(command, "sb") == 0){}
    }

    return 0;
}

int start(char str){
    plid = strtok(str, " \t\n");
    
}