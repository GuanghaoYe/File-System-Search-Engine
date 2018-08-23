# File Based Search Engine

## Make the project
run `make` to make the project
run `make clean` to clear all binary files

## Try in-memory version
```
./hw2/searchshell <docroot> [-s]
```
where `<docroot>` is an absolute or relative path to a directory to build an index under.
`[-s]` is an optional flag for remove the stop words

## Build the indexes
```
./hw3/buildfileindex crawlrootdir indexfilename
```
where:
  crawlrootdir is the name of a directory to crawl
  indexfilename is the name of the index file to create

## Run search engine in terminal
```
./hw3/filesearchshell [index files+]
```

## Run the web version
```
./hw4/http333d port staticfiles_directory [indices+]
```
