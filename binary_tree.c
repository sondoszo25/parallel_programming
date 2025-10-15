#include "binary_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

void LockRead(TreeNode *node)
{
    omp_set_lock(&node->locknode);
    node->num_of_readers++;
    if (node->num_of_readers == 1)
    {
        omp_set_lock(&node->writelocknode);
    }
    omp_unset_lock(&node->locknode);
}

void UnLockRead(TreeNode *node)
{
    omp_set_lock(&node->locknode);
    node->num_of_readers--;
    if (node->num_of_readers == 0)
    {
        omp_unset_lock(&node->writelocknode);
    }
    omp_unset_lock(&node->locknode);
}

void LockWrite(TreeNode *node)
{
    omp_set_lock(&node->writelocknode);
}

void UnLockWrite(TreeNode *node)
{
    omp_unset_lock(&node->writelocknode);
}

TreeNode *createNode(int data)
{
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    if (node)
    {
        node->data = data;
        node->left = node->right = NULL;
        omp_init_lock(&node->locknode);
        omp_init_lock(&node->writelocknode);
        node->num_of_readers = 0;
    }
    return node;
}
TreeNode *findMin(TreeNode *root)
{
    if (root == NULL)
        return NULL;

    TreeNode *current = root;
    LockRead(current);

    while (current->left != NULL)
    {
        TreeNode *next = current->left;
        LockRead(next);
        UnLockRead(current);
        current = next;
    }
    UnLockRead(current);
    return current;
}

TreeNode *insertNode(TreeNode *root, int data)
{
    if (root == NULL)
        return createNode(data);

    TreeNode *current = root;
    LockWrite(current);

    while (true)
    {
        if (data == current->data)
        {
            UnLockWrite(current);
            return root;
        }
        TreeNode **childPtr = (data < current->data) ? &current->left : &current->right;

        if (*childPtr != NULL)
        {
            LockWrite(*childPtr);
            UnLockWrite(current);
            current = *childPtr;
        }
        else
        {
            *childPtr = createNode(data);
            UnLockWrite(current);
            return root;
        }
    }
}





TreeNode *deleteNode(TreeNode *root, int data)
{
    if (root == NULL)
        return NULL;

    TreeNode *current = root;
    TreeNode *parent = NULL;

    LockWrite(current);

    while (current != NULL && current->data != data)
    {
        if (parent)
            UnLockWrite(parent);  

        parent = current;

        if (data < current->data)
            current = current->left;
        else
            current = current->right;

        if (current)
            LockWrite(current);  
        else
        {
            UnLockWrite(parent);
            return root;
        }
    }

    if (current == NULL)
    {
        if (parent)
            UnLockWrite(parent);
        return root;
    }

    if (current->left == NULL && current->right == NULL)
    {
        if (parent)
        {
            if (parent->left == current)
                parent->left = NULL;
            else
                parent->right = NULL;
        }
        else
        {
            root = NULL;
        }

        UnLockWrite(current);
        if (parent)
            UnLockWrite(parent);

        omp_destroy_lock(&current->locknode);
        omp_destroy_lock(&current->writelocknode);
        free(current);
    }
    else if (current->left == NULL || current->right == NULL)
    {
        TreeNode *child = (current->left) ? current->left : current->right;

        if (parent)
        {
            if (parent->left == current)
                parent->left = child;
            else
                parent->right = child;
        }
        else
        {
            root = child;
        }

        UnLockWrite(current);
        if (parent)
            UnLockWrite(parent);

        omp_destroy_lock(&current->locknode);
        omp_destroy_lock(&current->writelocknode);
        free(current);
    }
    else
    {
        TreeNode *parentOfSucc = current;
        TreeNode *succnode = current->right;

        LockWrite(succnode);

        while (succnode->left != NULL)
        {
            if (parentOfSucc != current)
                UnLockWrite(parentOfSucc);

            parentOfSucc = succnode;
            succnode = succnode->left;
            LockWrite(succnode);
        }

        current->data = succnode->data;

        if (parentOfSucc != current)
        {
            parentOfSucc->left = succnode->right;
            UnLockWrite(parentOfSucc);
        }
        else
        {
            current->right = succnode->right;
        }

        UnLockWrite(succnode);

        UnLockWrite(current);
        if (parent)
            UnLockWrite(parent);

        omp_destroy_lock(&succnode->locknode);
        omp_destroy_lock(&succnode->writelocknode);
        free(succnode);
    }

    return root;
}



bool searchNode(TreeNode *root, int data)
{
    TreeNode *current = root;
    if (current == NULL)
        return false;

    LockRead(current);

    while (current != NULL)
    {
        if (current->data == data)
        {
            UnLockRead(current);
            return true;
        }

        TreeNode *next = NULL;

        if (data < current->data)
            next = current->left;
        else
            next = current->right;

        if (next != NULL)
            LockRead(next); 

        UnLockRead(current);
        current = next;
    }

    return false;
}



void inorderTraversal(TreeNode *root)
{
    if (root == NULL)
        return;

    LockRead(root);

    inorderTraversal(root->left);

    printf("%d ", root->data);

    inorderTraversal(root->right);

    UnLockRead(root);
}

void freeTree(TreeNode *root)
{
    if (root == NULL)
        return;
    freeTree(root->left);
    freeTree(root->right);
    omp_destroy_lock(&root->locknode);
    omp_destroy_lock(&root->writelocknode);
    free(root);
}

void preorderTraversal(TreeNode *root)
{
    if (root == NULL)
        return;

    LockRead(root);

    printf("%d ", root->data);

    preorderTraversal(root->left);
    preorderTraversal(root->right);

    UnLockRead(root);
}

void postorderTraversal(TreeNode *root)
{
    if (root == NULL)
        return;

    LockRead(root);

    postorderTraversal(root->left);
    postorderTraversal(root->right);

    printf("%d ", root->data);

    UnLockRead(root);
}
