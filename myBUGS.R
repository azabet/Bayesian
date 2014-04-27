###########################################################################################
# This function calls OpenBUGS and executes commands written in "bugsCmds.txt"
###########################################################################################
myBUGS <- function(modelFile, data, param, numChains=3, nBurnin=1000, nIter=1000, nThin=1,
			inits=NULL, DIC=TRUE, over.relax=FALSE, digits=5, seed=NULL,
			BUGS.dir="C:/Program Files/OpenBUGS/OpenBUGS322/")
{
	# Create the BUGS commands file
	write.cmds.file(modelFile, param, numChains=numChains, nBurnin=nBurnin, nIter=nIter, 
			nThin=nThin, inits=inits, DIC=DIC, over.relax=over.relax, digits=digits, seed=seed)

	# Create the data file
	bugsData(data)

	# Set working directories
	bugsCmdFile <- file.path(getwd(), "bugsCmds.txt")
	bugsOutput <- file.path(getwd(), "bugsOutput.txt")
	if (file.exists(bugsOutput)) file.remove(bugsOutput)
	OpenBUGS <- file.path(BUGS.dir, "OpenBUGS.exe")

	# Call OpenBUGS
	cmd <- paste0("\"", OpenBUGS, "\" /PAR \"", bugsCmdFile, "\"", " /HEADLESS")
	cat("Starting OpenBUGS...")
	ok <- system(cmd) == 0
	if (!ok) stop("backBUGS failed to run BUGS commands")
	cat("Done\n")
	
	# Create an output
	out <- read.table("bugsOutput.txt", skip=9, header=T, nrows=length(param))
	return(out)
}

###########################################################################################
# This function creates a file "bugsCmds.txt" which contains BUGS commands to be 
# executed by OpenBUGS
###########################################################################################
write.cmds.file <- function(modelFile, param, numChains=3, nBurnin=1000, nIter=1000, nThin=1,
				inits=NULL, DIC=TRUE, over.relax=FALSE, digits=5, seed=NULL)
{
	# Create the commands file
	cmds.file <- file.path(getwd(), "bugsCmds.txt")
  	sink(cmds.file)

	# Write commands
	cat("modelDisplay('log')\n")
	cat("modelPrecision(", digits, ")\n", sep="")
	cat("modelSetWD('", getwd(), "')\n", sep="")
	cat("modelCheck('", modelFile, "')\n", sep="")
	cat("modelData('data.txt')\n")
	cat("modelCompile(",numChains,")\n", sep="")
  	if(!is.null(seed)) cat("modelSetRN(",seed,")\n", sep="")
  	if(!is.null(inits)) cat("modelInits(",inits,")\n", sep="")
	cat("modelGenInits()\n")
	cat("samplesSetThin(",nThin,")\n", sep="")
	cat("modelUpdate(",nBurnin,",",over.relax,")\n", sep="")
	if(DIC) cat("dicSet()\n")
	for (i in 1:length(param)) {cat("samplesSet('",param[i],"')\n", sep="")}
	cat("modelUpdate(",nIter,",",over.relax,")\n", sep="")
	cat("samplesStats('*')\n")
	#cat("samplesCoda('*')\n")
	if(DIC) {
		cat("dicStats()\n")
		cat("dicClear()\n")
	}
	cat("modelSaveLog('bugsOutput.txt')\n")
	cat("modelQuit('yes')\n")

	# Close the file
	sink()
}

###########################################################################################
# Following functions are copied from the BRUGS package for creating the data file "data.txt"
###########################################################################################
"bugsData" <- function(data, format="E", digits=5)
{#Write data file
	fileName <- file.path(getwd(), "data.txt")
	if (is.character(unlist(data))) { 
      	data.list <- lapply(as.list(data), get, pos=parent.frame(2))
		names(data.list) <- as.list(data)
		write.datafile(lapply(data.list, formatC, digits=digits, format=format), fileName)
	} else if(is.list(data)) { 
      	data <- lapply(data, function(x){x <- if(is.character(x)||is.factor(x)) match(x, unique(x)) else x})
            write.datafile(lapply(data, formatC, digits = digits, format = format), fileName)
	} else stop("Expected a list of data, a list or vector of variable names")      
}
"write.datafile" <- function (datalist, towhere, fill=TRUE){
    cat(formatdata(datalist), file=towhere, fill=fill)
}
"formatdata" <- function (datalist){
    if (!is.list(datalist) || is.data.frame(datalist))
      stop("argument to formatdata() ", "must be a list")
    n <- length(datalist)
    datalist.string <- vector(n, mode = "list")
    datanames <- names(datalist)
    for (i in 1:n) {
      if (is.factor(datalist[[i]]))
        datalist[[i]] <- as.integer(datalist[[i]])
      datalist.string[[i]] <-
        if (length(datalist[[i]]) == 1)
          paste(names(datalist)[i],
                "=", as.character(datalist[[i]]), sep = "")
      else if (is.vector(datalist[[i]]) && length(datalist[[i]]) > 1)
        paste(names(datalist)[i],
              "=c(", paste(as.character(datalist[[i]]), collapse = ", "),
              ")", sep = "")
      else
        paste(names(datalist)[i],
              "= structure(.Data= c(",
              paste(as.character(as.vector(aperm(datalist[[i]]))), collapse = ", "),
              "), .Dim=c(",
              paste(as.character(dim(datalist[[i]])), collapse = ", "),
              "))", sep = "")
    }
    datalist.tofile <- paste("list(",
                             paste(unlist(datalist.string), collapse = ", "),
                             ")", sep = "")
    return(datalist.tofile)
}
###########################################################################################
