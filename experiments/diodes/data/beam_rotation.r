#!/usr/bin/env Rscript

library(plyr)
library(ggplot2)

ddata <- read.csv("raw_data/20230606.1318_ten_runs_3_3_V_10k_ohms_diagonal.txt", header=F, skip=12)
ddata <- ddata[1:25000,]
names(ddata) <- c("run","wavelength","bus_voltage","pullup","x","y","measure","logic_level")

ppdata <- read.csv("raw_data/20230606.1554_ten_runs_3_3_V_10k_ohms_perpendicular.txt", header=F, skip=12)
ppdata <- ppdata[1:25000,]
names(ppdata) <- c("run","wavelength","bus_voltage","pullup","x","y","measure","logic_level")

padata <- read.csv("raw_data/20230606.1819_ten_runs_3_3_V_10k_ohms_parallel.txt", header=F, skip=12)
padata <- padata[1:25000,]
names(padata) <- c("run","wavelength","bus_voltage","pullup","x","y","measure","logic_level")

countz <- function(c)length(which(c==0))

ddata.zeroes <- ddply(ddata, .(run), summarize, high=countz(logic_level), orient="diagonal")
ppdata.zeroes <- ddply(ppdata, .(run), summarize, high=countz(logic_level), orient="perpendicular")
padata.zeroes <- ddply(padata, .(run), summarize, high=countz(logic_level), orient="parallel")

zeroes <- rbind(ddata.zeroes, ppdata.zeroes, padata.zeroes)
bardata <- ddply(zeroes, .(orient), summarize, mean=mean(high), sd=sd(high))

pdf("beam_rotation.pdf", 4, 4)

ggplot(bardata) + geom_col(aes(orient, mean, fill=orient)) + geom_errorbar(aes(x=orient, ymin=mean-sd, ymax=mean+sd), width=.1) + xlab("") + ylab("Active area size") + theme_bw() + guides(fill="none")
